/*
 * Cached MP4 playback through the PS Vita hardware decoder (SceAvPlayer).
 *
 * Mirrors the proven local-file path used by VitaDB Downloader: native file
 * I/O first, five physically contiguous video buffers and GXM NV12 sampling.
 * The network download finishes before the decoder opens the local file.
 */
#include "video.h"
#include "fsutil.h"
#include "net.h"

#include <psp2/avplayer.h>
#include <psp2/audioout.h>
#include <psp2/gxm.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/sysmodule.h>
#include <malloc.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <vita2d.h>

#define VID_DIR WS_CACHE "/video"
#define VIDEO_BUFFERS 5
#define FRAME_ALIGN (1024 * 1024)
#define VID_DOWNLOAD_READY (VID_LOADING + 100)

static volatile int state = VID_OFF;
static char err[160];
static char path[256], url_[512];
static NetProgress pr;
static SceUID dl_thread = -1, video_audio_thread = -1;
static volatile int video_audio_run;
static int decoder_mode = -1, decoder_errors[3];
static uint64_t decoder_started_us;
static vita2d_texture *cpu_frame;
static int cpu_w, cpu_h;
/* A second diagnostic path outside /logs, so copying just the root
 * ux0:data/WolffsStore also captures the exact AvPlayer failure. */
#define VIDEO_TRACE WS_DATA "/video_diagnostic.txt"

static SceAvPlayerHandle player;
static vita2d_texture *frames[VIDEO_BUFFERS]; /* GXM descriptor only, not owned frame data */
static int frame_slot, frame_w, frame_h, have_frame, ended;
static SceUID video_fd = -1;
static uint64_t video_size;


static void video_trace(const char *format, ...) {
    char line[480];
    va_list va;
    va_start(va, format);
    vsnprintf(line, sizeof(line), format, va);
    va_end(va);
    ws_log("video: %s", line);
    fs_mkdir_p(WS_DATA);
    SceUID fd = sceIoOpen(VIDEO_TRACE, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
    if (fd >= 0) {
        sceIoWrite(fd, line, strlen(line));
        sceIoWrite(fd, "\r\n", 2);
        sceIoClose(fd);
    }
}
static void video_trace_new_session(void) {
    fs_mkdir_p(WS_DATA);
    SceUID fd = sceIoOpen(VIDEO_TRACE, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd >= 0) sceIoClose(fd);
}
#define MAX_CDRAM 24
static struct { void *ptr; SceUID uid; } cdram[MAX_CDRAM];
static int cdram_count;
static void *cpu_frame_alloc(void *arg, uint32_t alignment, uint32_t bytes) {
    (void)arg;
    SceUID block = sceKernelAllocMemBlock("ws_cdram_video", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,
                                           bytes, NULL);
    if (block >= 0) {
        void *ptr = NULL;
        if (sceKernelGetMemBlockBase(block, &ptr) >= 0 && ptr &&
            cdram_count < MAX_CDRAM) {
            cdram[cdram_count].ptr = ptr;
            cdram[cdram_count++].uid = block;
            return ptr;
        }
        sceKernelFreeMemBlock(block);
    }
    video_trace("CDRAM frame allocation unavailable, trying CPU RAM (%u bytes)",bytes);
    return memalign(alignment < 16 ? 16 : alignment, bytes);
}
static void cpu_frame_free(void *arg, void *ptr) {
    (void)arg;
    if (!ptr) return;
    for (int i = 0; i < cdram_count; ++i) {
        if (cdram[i].ptr == ptr) {
            sceKernelFreeMemBlock(cdram[i].uid);
            cdram[i] = cdram[--cdram_count];
            return;
        }
    }
    free(ptr);
}
static inline uint8_t clamp_rgb(int x) {
    return (uint8_t)(x < 0 ? 0 : x > 255 ? 255 : x);
}
/* The hardware player returns two-plane YUV420 (NV12). CPU fallback
 * copies it to an owned vita2d ABGR8888 texture, avoiding GXM aliasing. */
static void nv12_to_rgba(const uint8_t *nv12, uint32_t *dst, int pitch, int w, int h) {
    const uint8_t *uv = nv12 + (size_t)w * h;
    for (int y = 0; y < h; ++y) {
        uint32_t *row = dst + (size_t)y * pitch;
        for (int x = 0; x < w; ++x) {
            int Y = nv12[(size_t)y * w + x], c = (y/2) * w + (x/2) * 2;
            int U = (int)uv[c] - 128, V = (int)uv[c+1] - 128;
            int R = clamp_rgb((Y * 298 + V * 409 + 128) >> 8);
            int G = clamp_rgb((Y * 298 - U * 100 - V * 208 + 128) >> 8);
            int B = clamp_rgb((Y * 298 + U * 516 + 128) >> 8);
            row[x] = 0xff000000u | ((uint32_t)B<<16) | ((uint32_t)G<<8) | (uint32_t)R;
        }
    }
}
static int video_audio_main(SceSize args, void *argp) {
    (void)args; (void)argp;
    int port = -1, oldrate = 0, oldch = 0, oldgrain = 0;
    while (video_audio_run) {
        SceAvPlayerFrameInfo frame;
        memset(&frame, 0, sizeof(frame));
        if (player > 0 && sceAvPlayerGetAudioData(player, &frame) && frame.pData) {
            int rate = frame.details.audio.sampleRate;
            int channels = frame.details.audio.channelCount;
            int grain = (int)(frame.details.audio.size / ((channels > 0 ? channels : 2) * 2));
            if (rate != oldrate || channels != oldch || grain != oldgrain) {
                if (port >= 0) sceAudioOutReleasePort(port);
                port = -1;
                oldrate = rate; oldch = channels; oldgrain = grain;
                if ((channels == 1 || channels == 2) && rate >= 8000 && rate <= 48000 &&
                    grain >= 64 && grain <= 2048)
                    port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, grain, rate,
                        channels == 1 ? SCE_AUDIO_OUT_MODE_MONO : SCE_AUDIO_OUT_MODE_STEREO);
                video_trace("Audio output: %d Hz, %d channels, %d frames, port=%d",
                            rate, channels, grain, port);
            }
            if (port >= 0) sceAudioOutOutput(port, frame.pData);
        } else sceKernelDelayThread(4000);
    }
    if (port >= 0) sceAudioOutReleasePort(port);
    return sceKernelExitDeleteThread(0);
}
static void stop_video_audio(void) {
    video_audio_run = 0;
    if (video_audio_thread >= 0) {
        sceKernelWaitThreadEnd(video_audio_thread, NULL, NULL);
        video_audio_thread = -1;
    }
}
static void start_video_audio(void) {
    video_audio_run = 1;
    video_audio_thread = sceKernelCreateThread("ws_avp_audio",video_audio_main,
                                              0x10000100,0x4000,0,0,NULL);
    if (video_audio_thread >= 0) sceKernelStartThread(video_audio_thread,0,NULL);
    else video_audio_run = 0;
}

static unsigned url_hash(const char *s) {
    unsigned h = 2166136261u;
    while (*s) { h ^= (unsigned char)*s++; h *= 16777619u; }
    return h;
}

/* AvPlayer allocates decoder textures in contiguous physical RAM. */
static void *mem_alloc(void *arg, uint32_t align, uint32_t size) {
    (void)arg;
    if (align < 16) align = 16;
    return memalign(align, size);
}
static void mem_free(void *arg, void *p) {
    (void)arg;
    free(p);
}

static void *gpu_alloc(void *arg, uint32_t align, uint32_t size) {
    (void)arg;
    uint32_t a = align > FRAME_ALIGN ? align : FRAME_ALIGN;
    size = (size + a - 1) & ~(a - 1);
    SceUID uid = sceKernelAllocMemBlock("ws_video", SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW, size, NULL);
    if (uid < 0) {
        ws_log("video: GPU memblock allocation failed 0x%08X (%u bytes)", uid, size);
        return NULL;
    }
    void *p = NULL;
    int r = sceKernelGetMemBlockBase(uid, &p);
    if (r < 0 || !p) {
        ws_log("video: GPU memblock base failed 0x%08X", r);
        sceKernelFreeMemBlock(uid);
        return NULL;
    }
    r = sceGxmMapMemory(p, size, SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE);
    if (r < 0) {
        ws_log("video: GXM map failed 0x%08X", r);
        sceKernelFreeMemBlock(uid);
        return NULL;
    }
    return p;
}
static void gpu_free(void *arg, void *p) {
    (void)arg;
    if (!p) return;
    /* The last frame may still be consumed by a GXM draw call. */
    vita2d_wait_rendering_done();
    SceUID uid = sceKernelFindMemBlockByAddr(p, 0);
    sceGxmUnmapMemory(p);
    if (uid >= 0) sceKernelFreeMemBlock(uid);
}

/* Fallback only: some firmware variants require an explicit ux0 file reader. */
static int file_open(void *arg, const char *filename) {
    (void)arg;
    video_fd = sceIoOpen(filename, SCE_O_RDONLY, 0);
    if (video_fd < 0) {
        ws_log("video: callback open failed 0x%08X: %s", video_fd, filename);
        return -1;
    }
    SceOff size = sceIoLseek(video_fd, 0, SCE_SEEK_END);
    if (size < 0) {
        sceIoClose(video_fd);
        video_fd = -1;
        return -1;
    }
    video_size = (uint64_t)size;
    sceIoLseek(video_fd, 0, SCE_SEEK_SET);
    return 0;
}
static int file_close(void *arg) {
    (void)arg;
    if (video_fd >= 0) sceIoClose(video_fd);
    video_fd = -1;
    video_size = 0;
    return 0;
}
static int file_read(void *arg, uint8_t *buffer, uint64_t pos, uint32_t length) {
    (void)arg;
    if (video_fd < 0 || pos > 0x7FFFFFFF) return -1;
    if (sceIoLseek(video_fd, (SceOff)pos, SCE_SEEK_SET) < 0) return -1;
    return sceIoRead(video_fd, buffer, length);
}
static uint64_t file_size(void *arg) {
    (void)arg;
    return video_size;
}

/* mode: 0 = native-file + contiguous GPU; 1 = callback-file + contiguous
 * GPU; 2 = callback-file + CDRAM/CPU RGBA fallback (FMVita-style). */
static int open_decoder(int mode) {
    SceAvPlayerInitData init;
    memset(&init, 0, sizeof(init));
    init.memoryReplacement.allocate = mem_alloc;
    init.memoryReplacement.deallocate = mem_free;
    init.memoryReplacement.allocateTexture = mode == 2 ? cpu_frame_alloc : gpu_alloc;
    init.memoryReplacement.deallocateTexture = mode == 2 ? cpu_frame_free : gpu_free;
    init.basePriority = 0xA0;
    init.numOutputVideoFrameBuffers = mode == 2 ? 8 : VIDEO_BUFFERS;
    init.autoStart = SCE_TRUE;
    if (mode != 0) {
        init.fileReplacement.open = file_open;
        init.fileReplacement.close = file_close;
        init.fileReplacement.readOffset = file_read;
        init.fileReplacement.size = file_size;
    }
    const char *names[] = {"native-GXM", "callback-GXM", "callback-CPU"};
    SceAvPlayerHandle handle = sceAvPlayerInit(&init);
    if (handle <= 0) {
        decoder_errors[mode] = (int)handle;
        video_trace("%s: sceAvPlayerInit error 0x%08X",names[mode],(int)handle);
        return 0;
    }
    int r = sceAvPlayerAddSource(handle, path);
    video_trace("%s: AddSource=0x%08X (file=%s)",names[mode],r,path);
    if (r < 0) {
        decoder_errors[mode] = r;
        sceAvPlayerClose(handle);
        file_close(NULL);
        return 0;
    }
    decoder_mode = mode;
    decoder_errors[mode] = 0;
    decoder_started_us = sceKernelGetSystemTimeWide();
    player = handle;
    start_video_audio();
    return 1;
}
static int try_decoder_from(int first) {
    for (int mode = first; mode < 3; ++mode)
        if (open_decoder(mode)) {
            video_trace("decoder active mode=%d",mode);
            return 1;
        }
    return 0;
}
static void start_player(void) {
    ended = have_frame = frame_slot = frame_w = frame_h = 0;
    decoder_mode = -1;
    memset(decoder_errors,0,sizeof(decoder_errors));
    if (!try_decoder_from(0)) {
        player = 0;
        snprintf(err, sizeof(err), "AvPlayer N:%08X C:%08X CPU:%08X. Veja video_diagnostic.txt",
                (unsigned)decoder_errors[0],(unsigned)decoder_errors[1],
                (unsigned)decoder_errors[2]);
        video_trace("All decoder paths failed: %s",err);
        state = VID_ERROR;
        return;
    }
    state = VID_PLAYING;
}

static int dl_main(SceSize args, void *argp) {
    (void)args;
    (void)argp;
    char part[280];
    snprintf(part, sizeof(part), "%s.part", path);
    int r = net_download(url_, part, &pr);
    if (pr.cancel) {
        sceIoRemove(part);
    } else if (r != NET_OK) {
        snprintf(err, sizeof(err), "%s", net_error());
        sceIoRemove(part);
        video_trace("Download failed: %s (%s)", err, url_);
        state = VID_ERROR;
    } else {
        sceIoRemove(path);
        int renamed = sceIoRename(part, path);
        if (renamed < 0) {
            snprintf(err, sizeof(err), "Nao foi possivel salvar o video no cache (0x%08X).", renamed);
            ws_log("video: rename failed 0x%08X (%s)", renamed, path);
            sceIoRemove(part);
            state = VID_ERROR;
        } else {
            video_trace("Downloaded MP4 to cache (%s)",path);
            state = VID_DOWNLOAD_READY;
        }
    }
    return sceKernelExitDeleteThread(0);
}

static void start_download(void) {
    state = VID_LOADING;
    dl_thread = sceKernelCreateThread("ws_vid_dl", dl_main, 0x10000100, 0x10000, 0, 0, NULL);
    if (dl_thread < 0) {
        snprintf(err, sizeof(err), "Falha ao criar thread de download (0x%08X).", dl_thread);
        ws_log("video: %s", err);
        state = VID_ERROR;
    } else {
        int r = sceKernelStartThread(dl_thread, 0, NULL);
        if (r < 0) {
            snprintf(err, sizeof(err), "Falha ao iniciar download (0x%08X).", r);
            ws_log("video: %s", err);
            sceKernelDeleteThread(dl_thread);
            dl_thread = -1;
            state = VID_ERROR;
        }
    }
}

void video_open(const char *url, const char *slug) {
    if (state != VID_OFF) video_close();
    err[0] = 0;
    memset(&pr, 0, sizeof(pr));
    video_trace_new_session();
    fs_mkdir_p(VID_DIR);
    video_trace("Video request, trace written to ux0:data/WolffsStore/video_diagnostic.txt");
    snprintf(url_, sizeof(url_), "%s", url ? url : "");
    snprintf(path, sizeof(path), VID_DIR "/%s-%08X.mp4", slug ? slug : "post", url_hash(url_));
    video_trace("source=%s, cache=%s",url_,path);
    if (!url_[0]) {
        snprintf(err, sizeof(err), "Video sem URL.");
        state = VID_ERROR;
        return;
    }

    /* IsLoaded == 0 means success; loading an already loaded module can fail. */
    int loaded = sceSysmoduleIsLoaded(SCE_SYSMODULE_AVPLAYER);
    video_trace("avplayer isLoaded=%08X",loaded);
    if (loaded < 0) {
        int r = sceSysmoduleLoadModule(SCE_SYSMODULE_AVPLAYER);
        video_trace("avplayer load result=%08X",r);
    }

    SceIoStat st;
    if (sceIoGetstat(path, &st) >= 0 && st.st_size > 0) {
        video_trace("using video cache (%llu bytes)", (unsigned long long)st.st_size);
        start_player();
        if (state != VID_ERROR) return;
        /* A partially written old cache must not permanently block playback. */
        ws_log("video: invalid cache, downloading a fresh copy");
        sceIoRemove(path);
        err[0] = 0;
    }
    start_download();
}

void video_close(void) {
    if (state == VID_OFF) return;
    if (dl_thread >= 0) {
        pr.cancel = 1;
        sceKernelWaitThreadEnd(dl_thread, NULL, NULL);
        dl_thread = -1;
    }
    stop_video_audio();
    if (player > 0) {
        /* Flush GXM BEFORE the decoder is allowed to release frame memory. */
        vita2d_wait_rendering_done();
        sceAvPlayerStop(player);
        sceAvPlayerClose(player);
        player = 0;
    }
    file_close(NULL);
    vita2d_wait_rendering_done();
    for (int i = 0; i < VIDEO_BUFFERS; i++) {
        /* These wrappers only alias AvPlayer-owned decoder memory. */
        free(frames[i]);
        frames[i] = NULL;
    }
    if (cpu_frame) { vita2d_free_texture(cpu_frame); cpu_frame = NULL; }
    cpu_w = cpu_h = 0;
    decoder_mode = -1;
    have_frame = frame_w = frame_h = frame_slot = ended = 0;
    state = VID_OFF;
}

int video_state(void) { return state == VID_DOWNLOAD_READY ? VID_LOADING : state; }
float video_load_progress(void) { return pr.total > 0 ? (float)pr.done / (float)pr.total : 0; }
const char *video_error(void) { return err; }
unsigned long long video_time_ms(void) { return player > 0 ? sceAvPlayerCurrentTime(player) : 0; }

void video_toggle_pause(void) {
    if (player <= 0) return;
    if (state == VID_PLAYING) {
        int r = sceAvPlayerPause(player);
        if (r >= 0) state = VID_PAUSED;
        else ws_log("video: pause failed 0x%08X", r);
    } else if (state == VID_PAUSED) {
        if (ended) {
            int r = sceAvPlayerJumpToTime(player, 0);
            ws_log("video: replay jump 0x%08X", r);
            ended = 0;
        }
        int r = sceAvPlayerResume(player);
        if (r >= 0) state = VID_PLAYING;
        else ws_log("video: resume failed 0x%08X", r);
    }
}

void video_seek(int seconds) {
    if (player <= 0) return;
    long long t = (long long)sceAvPlayerCurrentTime(player) + seconds * 1000LL;
    if (t < 0) t = 0;
    int r = sceAvPlayerJumpToTime(player, (uint64_t)t);
    if (r < 0) ws_log("video: seek %d s failed 0x%08X", seconds, r);
}

static void try_fallback_if_stalled(void) {
    if (state != VID_PLAYING || have_frame || decoder_mode < 0 ||
        sceKernelGetSystemTimeWide() - decoder_started_us < 5000000ull) return;
    int next = decoder_mode + 1;
    video_trace("No video frame in 5 seconds, switching from decoder mode %d",decoder_mode);
    stop_video_audio();
    vita2d_wait_rendering_done();
    if (player > 0) {
        sceAvPlayerStop(player);
        sceAvPlayerClose(player);
        player = 0;
    }
    file_close(NULL);
    decoder_mode = -1;
    if (next >= 3 || !try_decoder_from(next)) {
        snprintf(err, sizeof(err), "Sem quadros decodificados (N:%08X C:%08X CPU:%08X)",
                 (unsigned)decoder_errors[0],(unsigned)decoder_errors[1],
                 (unsigned)decoder_errors[2]);
        video_trace("Timeout: %s",err);
        state = VID_ERROR;
    }
}
void video_draw(void) {
    if (state == VID_DOWNLOAD_READY) {
        if (dl_thread >= 0) {
            sceKernelWaitThreadEnd(dl_thread, NULL, NULL);
            dl_thread = -1;
        }
        start_player();
    }
    if (player <= 0) return;
    try_fallback_if_stalled();
    if (player <= 0) return;

    if (state == VID_PLAYING && have_frame && !sceAvPlayerIsActive(player)) {
        /* Do not seek every frame: AvPlayer may have reached EOF. */
        ended = 1;
        state = VID_PAUSED;
    }
    if (state == VID_PLAYING) {
        SceAvPlayerFrameInfo fi;
        memset(&fi, 0, sizeof(fi));
        if (sceAvPlayerGetVideoData(player, &fi) && fi.pData) {
            int w = (int)fi.details.video.width, h = (int)fi.details.video.height;
            if (w <= 0 || h <= 0 || w > 1920 || h > 1088 || (w & 1) || (h & 1)) {
                video_trace("Invalid decoded dimensions %dx%d",w,h);
            } else if (decoder_mode == 2) {
                if (!cpu_frame || cpu_w != w || cpu_h != h) {
                    if (cpu_frame) vita2d_free_texture(cpu_frame);
                    cpu_frame = vita2d_create_empty_texture_format(
                        w,h,SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
                    cpu_w = w; cpu_h = h;
                }
                if (cpu_frame) {
                    vita2d_wait_rendering_done();
                    uint32_t *dst=(uint32_t *)vita2d_texture_get_datap(cpu_frame);
                    int stride=(int)vita2d_texture_get_stride(cpu_frame)/4;
                    if (dst && stride >= w) {
                        nv12_to_rgba(fi.pData,dst,stride,w,h);
                        if (!have_frame) video_trace("First CPU fallback frame %dx%d",w,h);
                        have_frame=1;
                        frame_w=w; frame_h=h;
                    }
                } else {
                    video_trace("Unable to allocate CPU RGB texture %dx%d",w,h);
                    snprintf(err,sizeof(err),"Sem memoria para tela do video");
                    state=VID_ERROR;
                }
            } else {
                int next = (frame_slot + 1) % VIDEO_BUFFERS;
                if (!frames[next]) frames[next] = calloc(1, sizeof(*frames[next]));
                if (frames[next]) {
                    int result = sceGxmTextureInitLinear(&frames[next]->gxm_tex, fi.pData,
                                            SCE_GXM_TEXTURE_FORMAT_YVU420P2_CSC1,
                                            w, h, 0);
                    if (result >= 0) {
                        sceGxmTextureSetMinFilter(&frames[next]->gxm_tex, SCE_GXM_TEXTURE_FILTER_LINEAR);
                        sceGxmTextureSetMagFilter(&frames[next]->gxm_tex, SCE_GXM_TEXTURE_FILTER_LINEAR);
                        frame_slot = next;
                        frame_w = w; frame_h = h;
                        if (!have_frame) video_trace("First GXM frame %dx%d, mode=%d",w,h,decoder_mode);
                        have_frame = 1;
                    } else if (!have_frame) video_trace("GXM texture init error %08X",result);
                }
            }
        }
    }
    vita2d_texture *draw_frame = decoder_mode == 2 ? cpu_frame : frames[frame_slot];
    if (have_frame && draw_frame) {
        float k = 960.f / frame_w;
        if (frame_h * k > 544) k = 544.f / frame_h;
        float dw = frame_w * k, dh = frame_h * k;
        vita2d_draw_texture_scale(draw_frame, (960 - dw) / 2, (544 - dh) / 2, k, k);
    }
}
