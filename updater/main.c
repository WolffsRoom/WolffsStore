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

/* Animação simples enquanto instala: a patinha de pixels e uma barra correndo. */
static int anim(SceSize a, void *b) {
    (void)a;
    (void)b;
    static const char *paw[] = {
        "...##..##...", "...##..##...", ".#........#.", ".#..####..#.",
        "...######...", "..########..", "..########..", "...##..##...",
    };
    int f = 0;
    while (busy) {
        vita2d_start_drawing();
        vita2d_clear_screen();
        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 12; x++)
                if (paw[y][x] == '#') vita2d_draw_rectangle(432 + x * 8, 180 + y * 8, 8, 8, RGBA8(255, 255, 255, 255));
        vita2d_draw_rectangle(330, 300, 300, 4, RGBA8(46, 46, 46, 255));
        int k = (f / 4) % 15;
        vita2d_draw_rectangle(330 + k * 20, 292, 20, 20, RGBA8(255, 255, 255, 255));
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
