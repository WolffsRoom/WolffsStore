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
#include <psp2/gxm.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/sysmodule.h>
#include <malloc.h>
#include <stdio.h>
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
static SceUID dl_thread = -1;
static SceAvPlayerHandle player;
static vita2d_texture *frames[VIDEO_BUFFERS]; /* GXM descriptor only, not owned frame data */
static int frame_slot, frame_w, frame_h, have_frame, ended;
static SceUID video_fd = -1;
static uint64_t video_size;

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

static int open_decoder(int custom_reader) {
    SceAvPlayerInitData init;
    memset(&init, 0, sizeof(init));
    init.memoryReplacement.allocate = mem_alloc;
    init.memoryReplacement.deallocate = mem_free;
    init.memoryReplacement.allocateTexture = gpu_alloc;
    init.memoryReplacement.deallocateTexture = gpu_free;
    init.basePriority = 0xA0; /* Valid AvPlayer priority (cf. VitaDB), NOT 0x10000100. */
    init.numOutputVideoFrameBuffers = VIDEO_BUFFERS;
    init.autoStart = SCE_TRUE;

    if (custom_reader) {
        init.fileReplacement.open = file_open;
        init.fileReplacement.close = file_close;
        init.fileReplacement.readOffset = file_read;
        init.fileReplacement.size = file_size;
    }
    SceAvPlayerHandle candidate = sceAvPlayerInit(&init);
    if (candidate <= 0) {
        ws_log("video: %s AvPlayerInit failed 0x%08X", custom_reader ? "custom" : "native", candidate);
        return 0;
    }
    int r = sceAvPlayerAddSource(candidate, path);
    ws_log("video: %s AddSource = 0x%08X (%s)", custom_reader ? "custom" : "native", r, path);
    if (r < 0) {
        sceAvPlayerClose(candidate);
        file_close(NULL);
        return 0;
    }
    player = candidate;
    return 1;
}

static void start_player(void) {
    ended = have_frame = frame_slot = frame_w = frame_h = 0;
    /* VitaDB's local player uses native file access. Try it before callbacks. */
    if (!open_decoder(0) && !open_decoder(1)) {
        player = 0;
        snprintf(err, sizeof(err), "Falha no AvPlayer. MP4 H.264 Baseline + AAC recomendado. Veja o log.");
        ws_log("video: failed both decoder paths, source=%s, file=%s", url_, path);
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
        ws_log("video: download failed: %s (%s)", err, url_);
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
    fs_mkdir_p(VID_DIR);
    snprintf(url_, sizeof(url_), "%s", url ? url : "");
    snprintf(path, sizeof(path), VID_DIR "/%s-%08X.mp4", slug ? slug : "post", url_hash(url_));
    if (!url_[0]) {
        snprintf(err, sizeof(err), "Video sem URL.");
        state = VID_ERROR;
        return;
    }

    /* IsLoaded == 0 means success; loading an already loaded module can fail. */
    if (sceSysmoduleIsLoaded(SCE_SYSMODULE_AVPLAYER) < 0) {
        int r = sceSysmoduleLoadModule(SCE_SYSMODULE_AVPLAYER);
        if (r < 0) ws_log("video: sysmodule load 0x%08X; attempting playback anyway", r);
    }

    SceIoStat st;
    if (sceIoGetstat(path, &st) >= 0 && st.st_size > 0) {
        ws_log("video: using cache %s (%llu bytes)", path, (unsigned long long)st.st_size);
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

void video_draw(void) {
    if (state == VID_DOWNLOAD_READY) {
        if (dl_thread >= 0) {
            sceKernelWaitThreadEnd(dl_thread, NULL, NULL);
            dl_thread = -1;
        }
        start_player();
    }
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
            if (w <= 0 || h <= 0 || w > 1920 || h > 1088) {
                ws_log("video: invalid frame size %dx%d", w, h);
            } else {
                int next = (frame_slot + 1) % VIDEO_BUFFERS;
                if (!frames[next]) frames[next] = calloc(1, sizeof(*frames[next]));
                if (frames[next]) {
                    /* Never copy NV12 to CPU RAM; GXM converts it on sampling. */
                    int r = sceGxmTextureInitLinear(&frames[next]->gxm_tex, fi.pData,
                                                    SCE_GXM_TEXTURE_FORMAT_YVU420P2_CSC1,
                                                    w, h, 0);
                    if (r >= 0) {
                        sceGxmTextureSetMinFilter(&frames[next]->gxm_tex, SCE_GXM_TEXTURE_FILTER_LINEAR);
                        sceGxmTextureSetMagFilter(&frames[next]->gxm_tex, SCE_GXM_TEXTURE_FILTER_LINEAR);
                        frame_slot = next;
                        frame_w = w;
                        frame_h = h;
                        if (!have_frame) ws_log("video: first frame %dx%d", w, h);
                        have_frame = 1;
                    } else {
                        ws_log("video: GXM texture init failed 0x%08X", r);
                    }
                }
            }
        }
    }
    if (have_frame && frames[frame_slot]) {
        float k = 960.f / frame_w;
        if (frame_h * k > 544) k = 544.f / frame_h;
        float dw = frame_w * k, dh = frame_h * k;
        vita2d_draw_texture_scale(frames[frame_slot], (960 - dw) / 2, (544 - dh) / 2, k, k);
    }
}
