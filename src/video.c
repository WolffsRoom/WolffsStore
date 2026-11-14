/*
 * Reprodutor de vídeo com o sceAvPlayer do sistema.
 * Os quadros vêm em NV12 (YUV420 em 2 planos); o GXM converte para RGB direto no
 * sampler (formato YVU420P2_CSC1), então o quadro é desenhado como uma textura vita2d.
 * O áudio sai numa porta própria (tipo MAIN), em uma thread.
 */
#include "video.h"
#include "fsutil.h"
#include "net.h"

#include <psp2/audioout.h>
#include <psp2/avplayer.h>
#include <psp2/gxm.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/sysmodule.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <vita2d.h>

#define VID_DIR WS_CACHE "/video"

static volatile int state = VID_OFF;
static char err[160];
static char path[256], url_[512];
static NetProgress pr;
static SceUID dl_thread = -1, audio_thread = -1;
static SceAvPlayerHandle player;
static volatile int audio_run;
static vita2d_texture *frame_tex;
static int frame_w, frame_h;
static SceUID video_fd = -1;
static uint64_t video_size;

static unsigned url_hash(const char *s) {
    unsigned h = 2166136261u;
    while (*s) { h ^= (unsigned char)*s++; h *= 16777619u; }
    return h;
}

/* ------------------------------------------------------------------ memória dos quadros (GPU) */

#define PHYCONT_ALIGN (1024 * 1024)

static void *mem_alloc(void *arg, uint32_t align, uint32_t size) {
    (void)arg;
    return memalign(align < 16 ? 16 : align, size);
}
static void mem_free(void *arg, void *p) {
    (void)arg;
    free(p);
}

static void *gpu_alloc(void *arg, uint32_t align, uint32_t size) {
    (void)arg;
    (void)align;
    size = (size + PHYCONT_ALIGN - 1) & ~(PHYCONT_ALIGN - 1);
    SceUID uid = sceKernelAllocMemBlock("ws_video", SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW, size, NULL);
    if (uid < 0) return NULL;
    void *p = NULL;
    if (sceKernelGetMemBlockBase(uid, &p) < 0 || !p) {
        sceKernelFreeMemBlock(uid);
        return NULL;
    }
    if (sceGxmMapMemory(p, size, SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE) < 0) {
        sceKernelFreeMemBlock(uid);
        return NULL;
    }
    return p;
}

static void gpu_free(void *arg, void *p) {
    (void)arg;
    if (!p) return;
    SceUID uid = sceKernelFindMemBlockByAddr(p, 0);
    vita2d_wait_rendering_done();
    sceGxmUnmapMemory(p);
    if (uid >= 0) sceKernelFreeMemBlock(uid);
}

/* O leitor próprio é o caminho usado pelo VitaShell/FMVita para arquivos em ux0:. */
static int file_open(void *arg, const char *filename) {
    (void)arg;
    video_fd = sceIoOpen(filename, SCE_O_RDONLY, 0);
    if (video_fd < 0) return -1;
    video_size = sceIoLseek(video_fd, 0, SCE_SEEK_END);
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

static int file_read(void *arg, uint8_t *buffer, uint64_t position, uint32_t length) {
    (void)arg;
    if (video_fd < 0 || sceIoLseek(video_fd, position, SCE_SEEK_SET) < 0) return -1;
    return sceIoRead(video_fd, buffer, length);
}

static uint64_t file_size(void *arg) {
    (void)arg;
    return video_size;
}

/* ------------------------------------------------------------------ áudio */

static int audio_main(SceSize args, void *argp) {
    (void)args;
    (void)argp;
    int port = -1, grain = 0, rate = 0, ch = 0;
    while (audio_run) {
        SceAvPlayerFrameInfo fi;
        memset(&fi, 0, sizeof(fi));
        if (state == VID_PLAYING && player > 0 && sceAvPlayerGetAudioData(player, &fi) && fi.pData) {
            int r = fi.details.audio.sampleRate, c = fi.details.audio.channelCount;
            int g = (int)(fi.details.audio.size / (c > 0 ? c * 2 : 4));
            if (port < 0 || r != rate || c != ch || g != grain) {
                if (port >= 0) sceAudioOutReleasePort(port);
                rate = r;
                ch = c;
                grain = g;
                port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, grain, rate,
                                           ch == 1 ? SCE_AUDIO_OUT_MODE_MONO : SCE_AUDIO_OUT_MODE_STEREO);
            }
            if (port >= 0) sceAudioOutOutput(port, fi.pData);
        } else {
            sceKernelDelayThread(2000);
        }
    }
    if (port >= 0) sceAudioOutReleasePort(port);
    return sceKernelExitDeleteThread(0);
}

/* ------------------------------------------------------------------ início / fim */

static void start_player(void) {
    SceAvPlayerInitData in;
    memset(&in, 0, sizeof(in));
    in.memoryReplacement.allocate = mem_alloc;
    in.memoryReplacement.deallocate = mem_free;
    in.memoryReplacement.allocateTexture = gpu_alloc;
    in.memoryReplacement.deallocateTexture = gpu_free;
    in.fileReplacement.open = file_open;
    in.fileReplacement.close = file_close;
    in.fileReplacement.readOffset = file_read;
    in.fileReplacement.size = file_size;
    in.basePriority = 0x10000100;
    in.numOutputVideoFrameBuffers = 5;
    in.autoStart = SCE_TRUE;
    player = sceAvPlayerInit(&in);
    int add = player > 0 ? sceAvPlayerAddSource(player, path) : (int)player;
    ws_log("video: callback init/add 0x%08X/0x%08X", (int)player, add);
    if (player > 0 && add < 0) {
        sceAvPlayerClose(player);
        file_close(NULL);
        memset(&in.fileReplacement, 0, sizeof(in.fileReplacement));
        player = sceAvPlayerInit(&in);
        add = player > 0 ? sceAvPlayerAddSource(player, path) : (int)player;
        ws_log("video: native init/add 0x%08X/0x%08X", (int)player, add);
    }
    if (player <= 0 || add < 0) {
        if (player > 0) sceAvPlayerClose(player);
        file_close(NULL);
        player = 0;
        snprintf(err, sizeof(err), "O Vita não conseguiu abrir o vídeo (use MP4 H.264 + AAC, até 960x544).");
        ws_log("video: init/add 0x%08X, path=%s, url=%s", add, path, url_);
        state = VID_ERROR;
        return;
    }
    audio_run = 1;
    audio_thread = sceKernelCreateThread("ws_vid_audio", audio_main, 0x10000100, 0x4000, 0, 0, NULL);
    if (audio_thread >= 0) sceKernelStartThread(audio_thread, 0, NULL);
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
        ws_log("video: %s (%s)", err, url_);
        state = VID_ERROR;
    } else {
        sceIoRemove(path);
        sceIoRename(part, path);
        state = VID_LOADING + 100; /* pronto para tocar: a thread principal inicia o player */
    }
    dl_thread = -1;
    return sceKernelExitDeleteThread(0);
}

void video_open(const char *url, const char *slug) {
    if (state != VID_OFF) video_close();
    static int mod;
    if (!mod) mod = sceSysmoduleLoadModule(SCE_SYSMODULE_AVPLAYER) >= 0 ? 1 : -1;
    err[0] = 0;
    memset(&pr, 0, sizeof(pr));
    fs_mkdir_p(VID_DIR);
    snprintf(url_, sizeof(url_), "%s", url);
    snprintf(path, sizeof(path), VID_DIR "/%s-%08X.mp4", slug, url_hash(url_));
    if (mod < 0) {
        snprintf(err, sizeof(err), "Módulo de vídeo do sistema indisponível.");
        ws_log("video: %s (%s)", err, url_);
        state = VID_ERROR;
        return;
    }
    if (fs_exists(path)) { /* já no cache */
        start_player();
        return;
    }
    state = VID_LOADING;
    dl_thread = sceKernelCreateThread("ws_vid_dl", dl_main, 0x10000100, 0x10000, 0, 0, NULL);
    if (dl_thread < 0 || sceKernelStartThread(dl_thread, 0, NULL) < 0) {
        snprintf(err, sizeof(err), "Não deu para iniciar o download do vídeo.");
        ws_log("video: %s (%s)", err, url_);
        state = VID_ERROR;
    }
}

void video_close(void) {
    if (state == VID_OFF) return;
    if (dl_thread >= 0) {
        pr.cancel = 1;
        sceKernelWaitThreadEnd(dl_thread, NULL, NULL);
        dl_thread = -1;
    }
    audio_run = 0;
    if (audio_thread >= 0) {
        sceKernelWaitThreadEnd(audio_thread, NULL, NULL);
        audio_thread = -1;
    }
    if (player > 0) {
        sceAvPlayerStop(player);
        vita2d_wait_rendering_done(); /* a GPU pode estar lendo o último quadro */
        sceAvPlayerClose(player);
        player = 0;
    }
    file_close(NULL);
    if (frame_tex) {
        vita2d_free_texture(frame_tex);
        frame_tex = NULL;
    }
    state = VID_OFF;
}

int video_state(void) { return state > VID_ERROR ? VID_LOADING : state; }
float video_load_progress(void) { return pr.total > 0 ? (float)pr.done / (float)pr.total : 0; }
const char *video_error(void) { return err; }
unsigned long long video_time_ms(void) { return player > 0 ? sceAvPlayerCurrentTime(player) : 0; }

void video_toggle_pause(void) {
    if (player <= 0) return;
    if (state == VID_PLAYING) {
        sceAvPlayerPause(player);
        state = VID_PAUSED;
    } else if (state == VID_PAUSED) {
        sceAvPlayerResume(player);
        state = VID_PLAYING;
    }
}

void video_seek(int seconds) {
    if (player <= 0) return;
    long long t = (long long)sceAvPlayerCurrentTime(player) + seconds * 1000LL;
    if (t < 0) t = 0;
    sceAvPlayerJumpToTime(player, (uint64_t)t);
}

void video_draw(void) {
    if (state == VID_LOADING + 100) start_player(); /* download terminou */
    if (player <= 0) return;
    if (state == VID_PLAYING && !sceAvPlayerIsActive(player)) { /* acabou: volta para o início, pausado */
        sceAvPlayerJumpToTime(player, 0);
    }
    SceAvPlayerFrameInfo fi;
    memset(&fi, 0, sizeof(fi));
    if (sceAvPlayerGetVideoData(player, &fi) && fi.pData) {
        int w = (int)fi.details.video.width, h = (int)fi.details.video.height;
        if (!frame_tex || w != frame_w || h != frame_h) {
            if (frame_tex) {
                vita2d_wait_rendering_done();
                vita2d_free_texture(frame_tex);
            }
            frame_tex = vita2d_create_empty_texture_format(16, 16, SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
            frame_w = w;
            frame_h = h;
        }
        if (frame_tex) {
            /* aponta a textura para o quadro decodificado (NV12, convertido pelo sampler) */
            vita2d_wait_rendering_done();
            sceGxmTextureInitLinear(&frame_tex->gxm_tex, fi.pData, SCE_GXM_TEXTURE_FORMAT_YVU420P2_CSC1, w, h, 0);
            sceGxmTextureSetMinFilter(&frame_tex->gxm_tex, SCE_GXM_TEXTURE_FILTER_LINEAR);
            sceGxmTextureSetMagFilter(&frame_tex->gxm_tex, SCE_GXM_TEXTURE_FILTER_LINEAR);
        }
    }
    if (frame_tex && frame_w > 0 && frame_h > 0) {
        float k = 960.f / frame_w;
        if (frame_h * k > 544) k = 544.f / frame_h;
        float dw = frame_w * k, dh = frame_h * k;
        vita2d_draw_texture_scale(frame_tex, (960 - dw) / 2, (544 - dh) / 2, k, k);
    }
}
