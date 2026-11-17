/*
 * Instalação de VPKs e extração de zips.
 * O fluxo de instalação (head.bin + promoter) é adaptado do installer.c do VitaSeed,
 * que por sua vez vem do package_installer.c do VitaShell (Copyright (C) 2015-2018,
 * TheFloW, GPLv3). Por isso o Wolff's Store também é distribuído sob a GPLv3.
 */
#include "install.h"
#include "fsutil.h"
#include "vendor/sha1.h"

#include <psp2/io/fcntl.h>
#include <psp2/promoterutil.h>
#include <psp2/sysmodule.h>
#include <archive.h>
#include <archive_entry.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define ntohl __builtin_bswap32

/* resources/head.bin embutido pelo CMake (ld -b binary) */
extern unsigned char _binary_resources_head_bin_start;
extern unsigned char _binary_resources_head_bin_end;

static char ierr[192];

static char last_tid[16];
const char *inst_error(void) { return ierr; }
const char *inst_last_titleid(void) { return last_tid; }

/* ------------------------------------------------------------------ zip */

static const char *strip_path(const char *name, const char **strip, int nstrip) {
    while (name[0] == '.' && name[1] == '/') name += 2;
    while (*name == '/') name++;
    int require_titleid = 0;
    for (int i = 0; i < nstrip; i++) {
        /* @PCSE00352/ searches the TitleID component anywhere within the
         * archive, e.g. Translation/PCSE00352/Pack or rePatch/PCSE.../data.
         * This also rejects unrelated files such as README or torrent. */
        const char *prefix = strip[i];
        if (prefix[0] == '@') {
            require_titleid = 1;
            const char *needle = prefix + 1;
            size_t n = strlen(needle);
            const char *segment = name;
            while (*segment) {
                if (!strncasecmp(segment, needle, n)) return segment + n;
                segment = strchr(segment, '/');
                if (!segment) break;
                segment++;
            }
        } else {
            size_t n = strlen(prefix);
            if (!strncasecmp(name, prefix, n)) return name + n;
        }
    }
    return require_titleid ? NULL : name;
}

static int count_entries(const char *zip, const char *password) {
    struct archive *a = archive_read_new();
    archive_read_support_format_zip(a);
    archive_read_support_format_rar(a);
    archive_read_support_format_rar5(a);
    if (password && *password) archive_read_add_passphrase(a, password);
    if (archive_read_open_filename(a, zip, 65536) != ARCHIVE_OK) {
        archive_read_free(a);
        return -1;
    }
    struct archive_entry *e;
    int n = 0;
    while (archive_read_next_header(a, &e) == ARCHIVE_OK) {
        n++;
        archive_read_data_skip(a);
    }
    archive_read_free(a);
    return n;
}

int inst_extract_password(const char *zip, const char *dst, const char **strip, int nstrip,
                          const char *password, NetProgress *pr) {
    if (pr) {
        pr->done = 0;
        pr->total = count_entries(zip, password);
    }
    struct archive *a = archive_read_new();
    archive_read_support_format_zip(a);
    archive_read_support_format_rar(a);
    archive_read_support_format_rar5(a);
    if (password && *password) archive_read_add_passphrase(a, password);
    if (archive_read_open_filename(a, zip, 256 * 1024) != ARCHIVE_OK) {
        snprintf(ierr, sizeof(ierr), "Arquivo ZIP/RAR invalido ou incompleto");
        archive_read_free(a);
        return -1;
    }
    static unsigned char buf[256 * 1024];
    struct archive_entry *e;
    int r, res = 0;
    while ((r = archive_read_next_header(a, &e)) == ARCHIVE_OK) {
        if (pr) {
            pr->done++;
            if (pr->cancel) {
                res = -2;
                break;
            }
        }
        const char *name = archive_entry_pathname(e);
        if (!name || strstr(name, "__MACOSX") || strstr(name, "..") || strchr(name, ':') || strchr(name, '\\')) {
            archive_read_data_skip(a);
            continue;
        }
        const char *rel = strip_path(name, strip, nstrip);
        if (!rel || !*rel) {
            archive_read_data_skip(a);
            continue;
        }
        char full[512];
        if (snprintf(full, sizeof(full), "%s/%s", dst, rel) >= (int)sizeof(full)) {
            snprintf(ierr, sizeof(ierr), "Caminho de arquivo longo demais no pacote");
            res = -1;
            break;
        }
        if (archive_entry_filetype(e) == AE_IFDIR) {
            fs_mkdir_p(full);
            continue;
        }
        if (archive_entry_filetype(e) != AE_IFREG) {
            archive_read_data_skip(a); /* ignora links e tipos especiais */
            continue;
        }
        char *slash = strrchr(full, '/');
        if (slash) {
            *slash = 0;
            fs_mkdir_p(full);
            *slash = '/';
        }
        SceUID fd = sceIoOpen(full, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
        if (fd < 0) {
            snprintf(ierr, sizeof(ierr), "não deu para gravar %s", rel);
            res = -1;
            break;
        }
        la_ssize_t n;
        while ((n = archive_read_data(a, buf, sizeof(buf))) > 0)
            if (sceIoWrite(fd, buf, (SceSize)n) != n) {
                n = -1;
                break;
            }
        sceIoClose(fd);
        if (n < 0) {
            snprintf(ierr, sizeof(ierr), "Falha no arquivo: senha, criptografia ou armazenamento (%s)", rel);
            res = -1;
            break;
        }
    }
    if (res == 0 && r != ARCHIVE_EOF) {
        snprintf(ierr, sizeof(ierr), "Arquivo compactado corrompido");
        res = -1;
    }
    archive_read_free(a);
    return res;
}

int inst_extract(const char *zip, const char *dst, const char **strip, int nstrip, NetProgress *pr) {
    return inst_extract_password(zip, dst, strip, nstrip, NULL, pr);
}

/* ------------------------------------------------------------------ head.bin (VitaShell) */

static void fpkg_hmac(const uint8_t *data, unsigned int len, uint8_t hmac[16]) {
    SHA1_CTX ctx;
    uint8_t sha1[20], buf[64];
    sha1_init(&ctx);
    sha1_update(&ctx, data, len);
    sha1_final(&ctx, sha1);
    memset(buf, 0, 64);
    memcpy(&buf[0], &sha1[4], 8);
    memcpy(&buf[8], &sha1[4], 8);
    memcpy(&buf[16], &sha1[12], 4);
    buf[20] = sha1[16];
    buf[21] = sha1[1];
    buf[22] = sha1[2];
    buf[23] = sha1[3];
    memcpy(&buf[24], &buf[16], 8);
    sha1_init(&ctx);
    sha1_update(&ctx, buf, 64);
    sha1_final(&ctx, sha1);
    memcpy(hmac, sha1, 16);
}

int inst_make_head_bin(const char *pkg) {
    char sfo[256], head[256], dir[256];
    snprintf(sfo, sizeof(sfo), "%s/sce_sys/param.sfo", pkg);
    snprintf(head, sizeof(head), "%s/sce_sys/package/head.bin", pkg);
    snprintf(dir, sizeof(dir), "%s/sce_sys/package", pkg);
    if (fs_exists(head)) return 0;
    char titleid[16] = "", contentid[48] = "";
    if (sfo_get(sfo, "TITLE_ID", titleid, sizeof(titleid)) < 0) {
        snprintf(ierr, sizeof(ierr), "VPK sem sce_sys/param.sfo");
        return -1;
    }
    sfo_get(sfo, "CONTENT_ID", contentid, sizeof(contentid));
    if (strlen(titleid) != 9) {
        snprintf(ierr, sizeof(ierr), "TITLE_ID inválido no param.sfo");
        return -1;
    }
    int size = (int)(&_binary_resources_head_bin_end - &_binary_resources_head_bin_start);
    uint8_t *hb = malloc(size);
    if (!hb) return -1;
    memcpy(hb, &_binary_resources_head_bin_start, size);
    char full[48];
    snprintf(full, sizeof(full), "EP9000-%s_00-0000000000000000", titleid);
    strncpy((char *)&hb[0x30], contentid[0] ? contentid : full, 48);
    uint8_t hmac[16];
    uint32_t len = ntohl(*(uint32_t *)&hb[0xD0]);
    fpkg_hmac(&hb[0], len, hmac);
    memcpy(&hb[len], hmac, 16);
    uint32_t off = ntohl(*(uint32_t *)&hb[0x8]);
    len = ntohl(*(uint32_t *)&hb[0x10]);
    uint32_t out = ntohl(*(uint32_t *)&hb[0xD4]);
    fpkg_hmac(&hb[off], len - 64, hmac);
    memcpy(&hb[out], hmac, 16);
    len = ntohl(*(uint32_t *)&hb[0xE8]);
    fpkg_hmac(&hb[0], len, hmac);
    memcpy(&hb[len], hmac, 16);
    fs_mkdir_p(dir);
    int r = fs_write(head, hb, size);
    free(hb);
    return r;
}

/* ------------------------------------------------------------------ promoter */

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

int inst_promote(const char *pkg) {
    int r = load_paf();
    ws_log("install: PAF -> 0x%08X", r);
    if (r < 0) goto fail;
    r = sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
    ws_log("install: PromoterUtil -> 0x%08X", r);
    if (r < 0) {
        unload_paf();
        goto fail;
    }
    r = scePromoterUtilityInit();
    ws_log("install: PromoterUtilityInit -> 0x%08X", r);
    if (r >= 0) {
        /* igual ao VitaShell: com Rif (licença gerada a partir do head.bin). Sem isso o
         * sistema recusava alguns VPKs (ex.: o Showcase da Godot 4 Vita). */
        r = scePromoterUtilityPromotePkgWithRif(pkg, 1);
        ws_log("install: PromotePkgWithRif(%s) -> 0x%08X", pkg, r);
        scePromoterUtilityExit();
    }
    sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
    unload_paf();
    if (r >= 0) return 0;
fail:
    snprintf(ierr, sizeof(ierr), "o sistema recusou a instalação (0x%08X)", r);
    ws_log("promoter falhou: 0x%08X (%s)", r, pkg);
    return r;
}

int inst_delete(const char *titleid) {
    int r = load_paf();
    ws_log("cleanup updater: PAF -> 0x%08X", r);
    if (r < 0) return r;
    r = sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
    if (r < 0) {
        unload_paf();
        return r;
    }
    r = scePromoterUtilityInit();
    if (r >= 0) {
        r = scePromoterUtilityDeletePkg(titleid);
        ws_log("cleanup updater: DeletePkg(%s) -> 0x%08X", titleid, r);
        scePromoterUtilityExit();
    }
    sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
    unload_paf();
    return r;
}

int inst_install_vpk(const char *vpk, NetProgress *pr, volatile int *phase) {
    fs_rm_rf(WS_PKG_TMP);
    fs_mkdir_p(WS_PKG_TMP);
    if (phase) *phase = 0; /* extraindo */
    int r = inst_extract(vpk, WS_PKG_TMP, NULL, 0, pr);
    if (r == 0) {
        char sfo[256];
        snprintf(sfo, sizeof(sfo), "%s/sce_sys/param.sfo", WS_PKG_TMP);
        if (!fs_exists(sfo)) {
            snprintf(ierr, sizeof(ierr), "VPK sem sce_sys/param.sfo");
            r = -1;
        }
    }
    if (r != 0) ws_log("install: extração falhou (%s): %s", vpk, ierr);
    if (r == 0) {
        r = inst_make_head_bin(WS_PKG_TMP);
        if (r != 0) ws_log("install: head.bin falhou: %s", ierr);
    }
    if (r == 0) {
        char sfo[256];
        snprintf(sfo, sizeof(sfo), "%s/sce_sys/param.sfo", WS_PKG_TMP);
        last_tid[0] = 0;
        sfo_get(sfo, "TITLE_ID", last_tid, sizeof(last_tid));
        if (phase) *phase = 1; /* instalando */
        ws_log("instalando %s (%s)", last_tid, vpk);
        r = inst_promote(WS_PKG_TMP);
        if (r == 0) ws_log("ok: ux0:app/%s", last_tid);
    }
    fs_rm_rf(WS_PKG_TMP);
    return r;
}
