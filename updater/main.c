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
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <stdio.h>
#include <string.h>

#define PKG "ux0:data/WolffsStore/pkg"
#define MAIN_TITLE "WOLF00001"

static volatile int busy = 1;

static const uint8_t *glyph5(char c) {
    static const uint8_t A[7]={14,17,17,31,17,17,17}, D[7]={30,17,17,17,17,17,30};
    static const uint8_t F[7]={31,16,16,30,16,16,16}, G[7]={14,17,16,23,17,17,14};
    static const uint8_t I[7]={31,4,4,4,4,4,31}, L[7]={16,16,16,16,16,16,31};
    static const uint8_t M[7]={17,27,21,21,17,17,17};
    static const uint8_t N[7]={17,25,25,21,19,19,17}, O[7]={14,17,17,17,17,17,14};
    static const uint8_t R[7]={30,17,17,30,20,18,17}, S[7]={15,16,16,14,1,1,30};
    static const uint8_t T[7]={31,4,4,4,4,4,4}, W[7]={17,17,17,21,21,27,17};
    static const uint8_t AP[7]={4,4,8,0,0,0,0}, BL[7]={0,0,0,0,0,0,0};
    switch (c) {
    case 'A': return A; case 'D': return D; case 'F': return F; case 'G': return G;
    case 'I': return I; case 'L': return L; case 'M': return M; case 'N': return N; case 'O': return O;
    case 'R': return R; case 'S': return S; case 'T': return T; case 'W': return W;
    case '\'': return AP; default: return BL;
    }
}

static void pixel_char(char c, float x, float y, int s, uint32_t color) {
    const uint8_t *g = glyph5(c);
    for (int r = 0; r < 7; r++)
        for (int k = 0; k < 5; k++)
            if (g[r] & (1 << (4 - k))) vita2d_draw_rectangle(x + k * s, y + r * s, s, s, color);
}

static float pixel_text_width(const char *s, int scale) { return strlen(s) * 6 * scale - scale; }

static void pixel_text(const char *s, float x, float y, int scale, uint32_t color) {
    for (int i = 0; s[i]; i++) pixel_char(s[i], x + i * 6 * scale, y, scale, color);
}

static void draw_paw(float x, float y, int scale) {
    static const char *paw[] = {
        "...##..##...", "...##..##...", ".#........#.", ".#..####..#.",
        "...######...", "..########..", "..########..", "...##..##...",
    };
    for (int r = 0; r < 8; r++)
        for (int k = 0; k < 12; k++)
            if (paw[r][k] == '#') vita2d_draw_rectangle(x + k * scale, y + r * scale, scale, scale, RGBA8(255, 255, 255, 255));
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
        draw_paw(426, 72, 9);
        const char *brand = "WOLFF'S ROOM";
        float bx = (960 - pixel_text_width(brand, 6)) / 2;
        for (int i = 0; brand[i]; i++) {
            int phase = (f / 3 - i * 2) % 36;
            if (phase < 0) phase += 36;
            float bounce = phase < 9 ? -(phase < 5 ? phase : 9 - phase) * 3 : 0;
            pixel_char(brand[i], bx + i * 36 + 4, 190 + bounce + 4, 6, RGBA8(46, 46, 46, 255));
            pixel_char(brand[i], bx + i * 36, 190 + bounce, 6, RGBA8(255, 255, 255, 255));
        }
        const char *status = "INSTALANDO";
        pixel_text(status, (960 - pixel_text_width(status, 3)) / 2, 286, 3, RGBA8(190, 190, 190, 255));
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
    sceAppMgrLaunchAppByUri(0xFFFFF, "psgm:play?titleid=" MAIN_TITLE);
    sceKernelDelayThread(50 * 1000);
    sceKernelExitProcess(0);
    return 0;
}
