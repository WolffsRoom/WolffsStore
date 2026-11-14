/*
 * Wolff's Store Updater (WOLF00002)
 * O app principal baixa a versão nova, extrai em ux0:data/WolffsStore/pkg (com head.bin),
 * instala este updater e o abre fechando a si mesmo. Aqui o Wolff's Store já está fechado,
 * então dá para instalar por cima e reabrir. Mesmo esquema do updater do VitaShell (GPLv3).
 */
#include <psp2/appmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/promoterutil.h>
#include <psp2/sysmodule.h>
#include <vita2d.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <stdio.h>
#include <string.h>

#define PKG "ux0:data/WolffsStore/pkg"
#define MAIN_TITLE "WOLF00001"

static volatile int busy = 1;
static vita2d_texture *brand_paw, *brand_font;

static vita2d_texture *load_wrt(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    unsigned char hd[8];
    if (fread(hd, 1, 8, f) != 8 || memcmp(hd, "WRT1", 4)) { fclose(f); return NULL; }
    int w = hd[4] | hd[5] << 8, h = hd[6] | hd[7] << 8;
    size_t n = (size_t)w * h * 4;
    unsigned char *px = malloc(n);
    if (!px || fread(px, 1, n, f) != n) { free(px); fclose(f); return NULL; }
    fclose(f);
    vita2d_texture *t = vita2d_create_empty_texture_format(w, h, SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ABGR);
    if (t) {
        unsigned char *dst = vita2d_texture_get_datap(t);
        unsigned stride = vita2d_texture_get_stride(t);
        for (int y = 0; y < h; y++) memcpy(dst + y * stride, px + (size_t)y * w * 4, (size_t)w * 4);
        vita2d_texture_set_filters(t, SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_POINT);
    }
    free(px);
    return t;
}

static float brand_width(const char *s, int scale) { return strlen(s) * 8 * scale; }

static void brand_char(char c, float x, float y, int scale, uint32_t color) {
    if (!brand_font || c == ' ') return;
    int g = (unsigned char)c - 32;
    vita2d_draw_texture_tint_part_scale(brand_font, x, y - 3 * scale,
        (g % 16) * 8, (g / 16) * 11, 8, 11, scale, scale, color);
}

static void brand_text(const char *s, float x, float y, int scale, uint32_t color) {
    for (int i = 0; s[i]; i++) brand_char(s[i], x + i * 8 * scale, y, scale, color);
}

static void draw_paw(float x, float y, float scale) {
    if (!brand_paw) return;
    float w = vita2d_texture_get_width(brand_paw), h = vita2d_texture_get_height(brand_paw);
    vita2d_draw_texture_tint_part_scale(brand_paw, x, y, 0, 0, w, h, scale, scale, RGBA8(255, 255, 255, 255));
}

static int load_paf(void) {
    /* igual ao VitaShell: o 4º argumento é {tamanho da estrutura, &resultado, -1, -1}.
     * Antes ia {1, &resultado, 0, 0} e o sistema recusava com 0x805A10FE (erro do sysmodule). */
    static uint32_t argp[] = {0x180000, -1, -1, 1, -1, -1};
    int result = -1;
    uint32_t buf[4];
    buf[0] = sizeof(buf);
    buf[1] = (uint32_t)&result;
    buf[2] = -1;
    buf[3] = -1;
    return sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, sizeof(argp), argp, (SceSysmoduleOpt *)buf);
}

static void unload_paf(void) {
    uint32_t buf[4] = {0, 0, 0, 0};
    sceSysmoduleUnloadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, 0, NULL, (SceSysmoduleOpt *)buf);
}

static int promote(const char *path) {
    int r = load_paf();
    if (r < 0) return r;
    r = sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
    if (r >= 0) {
        r = scePromoterUtilityInit();
        if (r >= 0) {
            r = scePromoterUtilityPromotePkgWithRif(path, 1); /* igual ao VitaShell */
            scePromoterUtilityExit();
        }
        sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
    }
    unload_paf();
    return r;
}

/* PromoterUtil não informa porcentagem: a barra é propositalmente indeterminada. */
static int anim(SceSize a, void *b) {
    (void)a;
    (void)b;
    int f = 0;
    while (busy) {
        vita2d_start_drawing();
        vita2d_clear_screen();
        draw_paw(412, 62, 2);
        const char *brand = "WOLFF'S ROOM";
        int scale = 4;
        float bx = (960 - brand_width(brand, scale)) / 2;
        for (int i = 0; brand[i]; i++) {
            int phase = (f / 3 - i * 2) % 36;
            if (phase < 0) phase += 36;
            float bounce = phase < 9 ? -(phase < 5 ? phase : 9 - phase) * 3 : 0;
            float cx = bx + i * 8 * scale;
            brand_char(brand[i], cx + 8, 214 + bounce + 8, scale, RGBA8(34, 34, 34, 255));
            brand_char(brand[i], cx + 4, 214 + bounce + 4, scale, RGBA8(74, 74, 74, 255));
            brand_char(brand[i], cx, 214 + bounce, scale, RGBA8(255, 255, 255, 255));
        }
        const char *status = "INSTALANDO";
        brand_text(status, (960 - brand_width(status, 2)) / 2, 294, 2, RGBA8(190, 190, 190, 255));
        vita2d_draw_rectangle(198, 354, 564, 36, RGBA8(255, 255, 255, 255));
        vita2d_draw_rectangle(204, 360, 552, 24, RGBA8(28, 28, 28, 255));
        int head = (f / 4) % 23;
        for (int i = 0; i < 23; i++) {
            int d = i - head;
            if (d < 0) d += 23;
            uint32_t c = d < 5 ? RGBA8(255 - d * 28, 255 - d * 28, 255 - d * 28, 255)
                               : RGBA8(54, 54, 54, 255);
            vita2d_draw_rectangle(208 + i * 24, 364, 18, 16, c);
        }
        vita2d_end_drawing();
        vita2d_swap_buffers();
        f++;
    }
    return 0;
}

int main(void) {
    sceAppMgrDestroyOtherApp();
    vita2d_init();
    vita2d_set_clear_color(RGBA8(6, 6, 6, 255));
    brand_paw = load_wrt("app0:data/tex/paw4.wrt");
    brand_font = load_wrt("app0:data/tex/font_title.wrt");
    SceUID th = sceKernelCreateThread("anim", anim, 0x10000100, 0x4000, 0, 0, NULL);
    if (th >= 0) sceKernelStartThread(th, 0, NULL);
    sceKernelDelayThread(300 * 1000);
    int r = promote(PKG);
    {   /* registro no mesmo log do app */
        char line[96];
        int n = snprintf(line, sizeof(line), "updater: instalação da atualização -> 0x%08X\r\n", r);
        sceIoMkdir("ux0:data/WolffsStore/logs", 0777);
        SceUID fd = sceIoOpen("ux0:data/WolffsStore/logs/wolffsstore.log", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
        if (fd >= 0) {
            sceIoWrite(fd, line, n);
            sceIoClose(fd);
        }
    }
    busy = 0;
    sceKernelDelayThread(100 * 1000);
    if (r < 0) {
        /* A failed promotion must never be advertised as an updated Store. */
        vita2d_start_drawing();
        vita2d_clear_screen();
        brand_text("FALHA AO INSTALAR", 210, 240, 3,
                   RGBA8(255, 125, 125, 255));
        vita2d_end_drawing();
        vita2d_swap_buffers();
        sceKernelDelayThread(3500 * 1000);
        sceKernelExitProcess(1);
    }
    /* This marker is consumed only AFTER the new Store has started.
     * Never uninstall WOLF00002 from inside its own updater process. */
    sceIoMkdir("ux0:data/WolffsStore", 0777);
    {
        SceUID fd = sceIoOpen("ux0:data/WolffsStore/updater_cleanup.pending",
                            SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
        if (fd >= 0) {
            sceIoWrite(fd, "1", 1);
            sceIoClose(fd);
        }
    }
    /* Wait for shell registration. VitaShell and PSVitaAlive use this single
     * queued URI + immediate process exit to avoid the "close Updater?"
     * prompt caused by switching games while the helper is still active. */
    sceKernelDelayThread(1600 * 1000);
    vita2d_wait_rendering_done();
    {
        char line[112];
        int n = snprintf(line, sizeof(line),
                         "updater: handing off to Store with shell URI\r\n");
        SceUID fd = sceIoOpen("ux0:data/WolffsStore/logs/wolffsstore.log",
                            SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
        if (fd >= 0) { sceIoWrite(fd, line, n); sceIoClose(fd); }
    }
    sceAppMgrLaunchAppByUri(0xFFFFF, "psgm:play?titleid=" MAIN_TITLE);
    sceKernelExitProcess(0);
    return 0;
}
