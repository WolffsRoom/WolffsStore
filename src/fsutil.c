#include "fsutil.h"

#include <psp2/io/devctl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/rtc.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int fs_exists(const char *path) {
    SceIoStat st;
    return sceIoGetstat(path, &st) >= 0;
}

int fs_is_dir(const char *path) {
    SceIoStat st;
    if (sceIoGetstat(path, &st) < 0) return 0;
    return SCE_S_ISDIR(st.st_mode);
}

void fs_mkdir_p(const char *path) {
    char tmp[512];
    snprintf(tmp, sizeof(tmp), "%s", path);
    int len = (int)strlen(tmp);
    if (len > 0 && tmp[len - 1] == '/') tmp[--len] = 0;
    for (int i = 1; tmp[i]; i++) {
        if (tmp[i] == '/' && tmp[i - 1] != ':') {
            tmp[i] = 0;
            sceIoMkdir(tmp, 0777);
            tmp[i] = '/';
        }
    }
    sceIoMkdir(tmp, 0777);
}

void fs_rm_rf(const char *path) {
    SceUID d = sceIoDopen(path);
    if (d < 0) {
        sceIoRemove(path);
        return;
    }
    SceIoDirent e;
    for (;;) {
        memset(&e, 0, sizeof(e));
        if (sceIoDread(d, &e) <= 0) break;
        if (!strcmp(e.d_name, ".") || !strcmp(e.d_name, "..")) continue;
        char child[512];
        snprintf(child, sizeof(child), "%s/%s", path, e.d_name);
        if (SCE_S_ISDIR(e.d_stat.st_mode)) fs_rm_rf(child);
        else sceIoRemove(child);
    }
    sceIoDclose(d);
    sceIoRmdir(path);
}

char *fs_read(const char *path, int *size) {
    SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
    if (fd < 0) return NULL;
    SceOff n = sceIoLseek(fd, 0, SCE_SEEK_END);
    sceIoLseek(fd, 0, SCE_SEEK_SET);
    if (n < 0 || n > 64 * 1024 * 1024) {
        sceIoClose(fd);
        return NULL;
    }
    char *buf = malloc((size_t)n + 1);
    if (!buf) {
        sceIoClose(fd);
        return NULL;
    }
    int r = sceIoRead(fd, buf, (SceSize)n);
    sceIoClose(fd);
    if (r != (int)n) {
        free(buf);
        return NULL;
    }
    buf[n] = 0;
    if (size) *size = (int)n;
    return buf;
}

int fs_write(const char *path, const void *buf, int size) {
    SceUID fd = sceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd < 0) return fd;
    int w = sceIoWrite(fd, buf, size);
    sceIoClose(fd);
    return w == size ? 0 : -1;
}

int fs_copy(const char *src, const char *dst) {
    SceUID in = sceIoOpen(src, SCE_O_RDONLY, 0);
    if (in < 0) return in;
    SceUID out = sceIoOpen(dst, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (out < 0) {
        sceIoClose(in);
        return out;
    }
    static char buf[64 * 1024];
    int r, res = 0;
    while ((r = sceIoRead(in, buf, sizeof(buf))) > 0)
        if (sceIoWrite(out, buf, r) != r) {
            res = -1;
            break;
        }
    sceIoClose(in);
    sceIoClose(out);
    return r < 0 ? r : res;
}

long long fs_free_space(const char *dev) {
    /* mesmo pedido que o VitaShell/PKGj usam (devctl 0x3001) */
    struct {
        uint64_t max_size;
        uint64_t free_size;
        uint32_t cluster_size;
        void *unk;
    } info;
    memset(&info, 0, sizeof(info));
    if (sceIoDevctl(dev, 0x3001, NULL, 0, &info, sizeof(info)) < 0) return -1;
    return (long long)info.free_size;
}

int fs_grep_ci(const char *path, const char *needle) {
    char *t = fs_read(path, NULL);
    if (!t) return 0;
    for (char *p = t; *p; p++) *p = (char)tolower((unsigned char)*p);
    char low[64];
    snprintf(low, sizeof(low), "%s", needle);
    for (char *p = low; *p; p++) *p = (char)tolower((unsigned char)*p);
    int found = strstr(t, low) != NULL;
    free(t);
    return found;
}

/* ------------------------------------------------------------------ param.sfo */

typedef struct {
    uint32_t magic, version, keyofs, valofs, count;
} __attribute__((packed)) SfoHeader;

typedef struct {
    uint16_t nameofs;
    uint8_t alignment, type;
    uint32_t valsize, totalsize, dataofs;
} __attribute__((packed)) SfoEntry;

int sfo_get(const char *sfo_path, const char *key, char *out, int out_size) {
    int size = 0;
    char *buf = fs_read(sfo_path, &size);
    if (!buf) return -1;
    SfoHeader *h = (SfoHeader *)buf;
    int res = -2;
    if (size >= (int)sizeof(SfoHeader) && h->magic == 0x46535000) {
        SfoEntry *e = (SfoEntry *)(buf + sizeof(SfoHeader));
        for (uint32_t i = 0; i < h->count; i++) {
            if (h->keyofs + e[i].nameofs >= (uint32_t)size) break;
            if (!strcmp(buf + h->keyofs + e[i].nameofs, key)) {
                snprintf(out, out_size, "%s", buf + h->valofs + e[i].dataofs);
                res = 0;
                break;
            }
        }
    }
    free(buf);
    return res;
}

int version_cmp(const char *a, const char *b) {
    int part = 0;
    while (*a || *b) {
        while (*a && !isdigit((unsigned char)*a)) a++;
        while (*b && !isdigit((unsigned char)*b)) b++;
        const char *as = a, *bs = b;
        long x = strtol(a, (char **)&a, 10), y = strtol(b, (char **)&b, 10);
        /* O histórico usa 0.41, 0.42... mas a versão pública seguinte é 0.5.
         * No componente menor, um único dígito representa a dezena:
         * 0.5 == 0.50, permitindo que 0.42 < 0.5 sem loops de update. */
        if (part == 1) {
            if (a - as == 1) x *= 10;
            if (b - bs == 1) y *= 10;
        }
        if (x != y) return x < y ? -1 : 1;
        if (!*a && !*b) break;
        part++;
    }
    return 0;
}

void utf8_to_latin1(const char *in, char *out, int n) {
    const unsigned char *p = (const unsigned char *)in;
    int o = 0;
    while (*p && o < n - 1) {
        unsigned c = *p++;
        if (c >= 0xC0 && (*p & 0xC0) == 0x80) {
            unsigned d = ((c & 0x1F) << 6) | (*p++ & 0x3F);
            if (c >= 0xE0) { /* 3 bytes: fora do Latin-1 */
                if ((*p & 0xC0) == 0x80) p++;
                d = '?';
            }
            c = d < 256 ? d : '?';
        }
        out[o++] = (char)c;
    }
    out[o] = 0;
}

char *dup_latin1(const char *utf8) {
    if (!utf8) utf8 = "";
    int n = (int)strlen(utf8) + 1;
    char *s = malloc(n);
    if (s) utf8_to_latin1(utf8, s, n);
    return s;
}

void ws_log(const char *fmt, ...) {
    char line[640];
    int n = 0;
    SceDateTime t;
    if (sceRtcGetCurrentClockLocalTime(&t) >= 0)
        n = snprintf(line, sizeof(line), "%04d-%02d-%02d %02d:%02d:%02d  ", t.year, t.month, t.day, t.hour, t.minute, t.second);
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line + n, sizeof(line) - n, fmt, ap);
    va_end(ap);
    fs_mkdir_p(WS_LOGS);
    const char *path = WS_LOGS "/wolffsstore.log";
    SceIoStat st;
    if (sceIoGetstat(path, &st) >= 0 && st.st_size > 256 * 1024) {
        sceIoRemove(WS_LOGS "/wolffsstore.old.log");
        sceIoRename(path, WS_LOGS "/wolffsstore.old.log");
    }
    SceUID fd = sceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
    if (fd < 0) return;
    sceIoWrite(fd, line, strlen(line));
    sceIoWrite(fd, "\r\n", 2);
    sceIoClose(fd);
}
