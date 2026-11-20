/*
 * Wolff's Store Updater (WFFS00043)
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
#include <math.h>

#define PKG "ux0:data/WolffsStore/pkg"
#define MAIN_TITLE "WFFS00042"

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

/* One-time migration of WOLF00001 to WFFS00042, performed only by the
 * updater (the old Store must no longer be running). Check the installed
 * SFO itself; never erase an unrelated app that happens to reuse the ID. */
#define LEGACY_APP "WOLF00001"
#define LEGACY_MIGRATION_MARKER "ux0:data/WolffsStore/legacy_id_migration_v0992.done"

static uint32_t sfo_u32(const unsigned char *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
           (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
static unsigned sfo_u16(const unsigned char *p) {
    return (unsigned)p[0] | (unsigned)p[1] << 8;
}
static int installed_sfo_value(const char *path, const char *field,
                               char *out, size_t cap) {
    unsigned char b[4096];
    if (!cap) return -1;
    out[0] = 0;
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    size_t size = fread(b, 1, sizeof(b), f);
    fclose(f);
    if (size < 20 || sfo_u32(b) != 0x46535000U) return -1;
    uint32_t key_base = sfo_u32(b + 8), value_base = sfo_u32(b + 12);
    uint32_t count = sfo_u32(b + 16);
    if (count > (size - 20) / 16) return -1;
    for (uint32_t i = 0; i < count; ++i) {
        const unsigned char *e = b + 20 + i * 16;
        size_t k = (size_t)key_base + sfo_u16(e);
        size_t v = (size_t)value_base + sfo_u32(e + 12);
        size_t len = sfo_u32(e + 4);
        if (k >= size || v >= size || len > size - v) continue;
        const char *key = (const char *)b + k;
        if (!memchr(key, '\0', size - k) || strcmp(key, field)) continue;
        size_t n = 0;
        while (n < len && b[v + n] && n + 1 < cap) ++n;
        memcpy(out, b + v, n);
        out[n] = '\0';
        return 0;
    }
    return -1;
}

static void migration_log(const char *text) {
    sceIoMkdir("ux0:data/WolffsStore/logs", 0777);
    SceUID fd = sceIoOpen("ux0:data/WolffsStore/logs/wolffsstore.log",
                         SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
    if (fd >= 0) {
        sceIoWrite(fd, text, (SceSize)strlen(text));
        sceIoClose(fd);
    }
}

static int valid_store_sfo(const char *path, const char *expected_id,
                           int check_legacy_version) {
    char id[32], title[96], version[32];
    if (installed_sfo_value(path, "TITLE_ID", id, sizeof(id)) < 0 ||
        installed_sfo_value(path, "TITLE", title, sizeof(title)) < 0 ||
        installed_sfo_value(path, "APP_VER", version, sizeof(version)) < 0)
        return 0;
    if (strcmp(id, expected_id) || strcmp(title, "Wolff's Store")) return 0;
    if (check_legacy_version) {
        unsigned major = 999, minor = 999;
        if (sscanf(version, "%u.%u", &major, &minor) != 2 ||
            major != 0 || minor > 99)
            return 0; /* never touch an unknown/future WOLF00001 install */
    }
    return 1;
}

static int delete_legacy_store(void) {
    int r = load_paf();
    if (r < 0) return r;
    r = sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
    if (r >= 0) {
        r = scePromoterUtilityInit();
        if (r >= 0) {
            r = scePromoterUtilityDeletePkg(LEGACY_APP);
            scePromoterUtilityExit();
        }
        sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
    }
    unload_paf();
    return r;
}

static void migrate_legacy_store_after_install(void) {
    FILE *f = fopen(LEGACY_MIGRATION_MARKER, "rb");
    if (f) { fclose(f); return; }
    const char *new_sfo = "ux0:app/WFFS00042/sce_sys/param.sfo";
    const char *old_sfo = "ux0:app/WOLF00001/sce_sys/param.sfo";
    /* Require the newly installed Store to be registered and correctly
     * identified before considering any removal of the previous bubble. */
    if (!valid_store_sfo(new_sfo, "WFFS00042", 0)) {
        migration_log("migration: new WFFS00042 identity not verified; preserving old app\r\n");
        return;
    }
    int legacy_valid = valid_store_sfo(old_sfo, LEGACY_APP, 1);
    if (legacy_valid) {
        migration_log("migration: WOLF00001 TITLE/ID/APP_VER verified, deleting old Store\r\n");
        int r = delete_legacy_store();
        char msg[120];
        snprintf(msg, sizeof(msg), "migration: DeletePkg(WOLF00001) -> 0x%08X\r\n", r);
        migration_log(msg);
        if (r < 0) return; /* retry only on a future updater run */
    } else {
        migration_log("migration: old WOLF00001 missing or identity/version mismatch; untouched\r\n");
    }
    f = fopen(LEGACY_MIGRATION_MARKER, "wb");
    if (f) {
        fputs("Migration checked for v0.99.2\r\n", f);
        fclose(f);
    }
}

/* PromoterUtil não informa porcentagem: a barra é propositalmente indeterminada. */
static int anim(SceSize a, void *b) {
    (void)a;
    (void)b;
    int f = 0;
    while (busy) {
        vita2d_start_drawing();
        vita2d_clear_screen();
        /* A 25% smaller paw, centered 22 px higher in the updater UI. */
        if (brand_paw) {
            /* Animated pulse and slight floating motion keep the logo alive. */
            const float scale = 1.5f + .06f * (1.f + sinf(f * .07f));
            float w = vita2d_texture_get_width(brand_paw);
            draw_paw((960.f - w * scale) / 2.f,
                     40.f - 3.f * sinf(f * .045f), scale);
        }
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
    /* Migrate ONLY the positively identified old WOLF00001 installation
     * after WFFS00042 has been promoted. The helper is separate so the
     * Store never uninstalls the legacy app from its own first session. */
    migrate_legacy_store_after_install();

    /* Keep this reusable updater installed, instead of asking Store to
     * delete it during its foreground session. The next upgrade overwrites
     * the helper in place without a destructive SceShell LiveArea refresh. */
    /* Wait for SceShell registration. For psgm:play URIs the VitaSDK
     * explicitly documents 0x20000 to launch the app. 0xFFFFF opens
     * LiveArea, which caused the observed return instead of reopening. */
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
    int launched = sceAppMgrLaunchAppByUri(0x20000, "psgm:play?titleid=" MAIN_TITLE);
    {
        char line[120];
        int n = snprintf(line, sizeof(line),
                         "updater: direct Store launch returned 0x%08X\r\n", launched);
        SceUID fd = sceIoOpen("ux0:data/WolffsStore/logs/wolffsstore.log",
                             SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
        if (fd >= 0) { sceIoWrite(fd, line, n); sceIoClose(fd); }
    }
    if (launched >= 0) sceKernelExitProcess(0);
    /* Keep the updater alive on launch failure for visible diagnosis. */
    return launched;
}
