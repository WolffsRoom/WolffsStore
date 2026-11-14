/* Rede: libcurl + OpenSSL sobre o SceNet, como no VitaSeed (src/http.c), que
 * explica por que o sceSsl nativo não serve para o GitHub. */
#include "net.h"

#include <psp2/io/fcntl.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/sysmodule.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NET_POOL (1024 * 1024)
#define UA "WolffsStore/" WS_VERSION_STR " (PS Vita)"

#ifndef WS_VERSION
#define WS_VERSION "0.2"
#endif
#define WS_VERSION_STR WS_VERSION

static int inited;
static char err[192];

const char *net_error(void) { return err; }

void net_init(void) {
    if (inited) return;
    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    static char *pool;
    pool = malloc(NET_POOL);
    if (!pool) {
        snprintf(err, sizeof(err), "sem memória para a rede");
        return;
    }
    SceNetInitParam p;
    p.memory = pool;
    p.size = NET_POOL;
    p.flags = 0;
    sceNetInit(&p);
    sceNetCtlInit();
    curl_global_init(CURL_GLOBAL_DEFAULT);
    inited = 1;
}

int net_online(int wait_ms) {
    if (!inited) return 0;
    for (int t = 0; t <= wait_ms; t += 100) {
        int st = 0;
        if (sceNetCtlInetGetState(&st) == 0 && st == SCE_NETCTL_STATE_CONNECTED) return 1;
        sceKernelDelayThread(100 * 1000);
    }
    return 0;
}

static void common(CURL *c, const char *url) {
    curl_easy_setopt(c, CURLOPT_URL, url);
    curl_easy_setopt(c, CURLOPT_USERAGENT, UA);
    curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(c, CURLOPT_MAXREDIRS, 8L);
    curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT, 20L);
    curl_easy_setopt(c, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(c, CURLOPT_FAILONERROR, 1L);
    /* conexão parada por 30 s = erro (downloads grandes não têm tempo total) */
    curl_easy_setopt(c, CURLOPT_LOW_SPEED_LIMIT, 1L);
    curl_easy_setopt(c, CURLOPT_LOW_SPEED_TIME, 30L);
}

typedef struct { char *buf; size_t len, cap; } Mem;

static size_t mem_cb(void *d, size_t s, size_t n, void *u) {
    Mem *m = u;
    size_t t = s * n;
    if (m->len + t + 1 > m->cap) {
        size_t nc = m->cap;
        while (m->len + t + 1 > nc) nc *= 2;
        char *nb = realloc(m->buf, nc);
        if (!nb) return 0;
        m->buf = nb;
        m->cap = nc;
    }
    memcpy(m->buf + m->len, d, t);
    m->len += t;
    m->buf[m->len] = 0;
    return t;
}

char *net_fetch(const char *url, int *size) {
    if (!inited) return NULL;
    CURL *c = curl_easy_init();
    if (!c) return NULL;
    Mem m = {malloc(65536), 0, 65536};
    if (!m.buf) {
        curl_easy_cleanup(c);
        return NULL;
    }
    m.buf[0] = 0;
    common(c, url);
    curl_easy_setopt(c, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(c, CURLOPT_TIMEOUT, 60L);
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, mem_cb);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, &m);
    CURLcode r = curl_easy_perform(c);
    curl_easy_cleanup(c);
    if (r != CURLE_OK) {
        snprintf(err, sizeof(err), "%s", curl_easy_strerror(r));
        free(m.buf);
        return NULL;
    }
    if (size) *size = (int)m.len;
    return m.buf;
}

/* ------------------------------------------------------------------ download para arquivo */

#define WBUF (512 * 1024) /* junta os pedaços antes de gravar no cartão */

typedef struct {
    SceUID fd;
    char *buf;
    int used, failed;
    NetProgress *pr;
    double t0, t_last;
    long long b_last;
} Dl;

static double now_s(void) { return sceKernelGetProcessTimeWide() / 1000000.0; }

static int flush(Dl *d) {
    if (d->used && sceIoWrite(d->fd, d->buf, d->used) != d->used) {
        d->failed = 1;
        return -1;
    }
    d->used = 0;
    return 0;
}

static size_t file_cb(void *data, size_t s, size_t n, void *u) {
    Dl *d = u;
    size_t t = s * n, off = 0;
    while (off < t) {
        size_t k = t - off;
        if (k > (size_t)(WBUF - d->used)) k = WBUF - d->used;
        memcpy(d->buf + d->used, (char *)data + off, k);
        d->used += (int)k;
        off += k;
        if (d->used == WBUF && flush(d) < 0) return 0;
    }
    return t;
}

static int prog_cb(void *u, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ut, curl_off_t un) {
    (void)ut;
    (void)un;
    Dl *d = u;
    NetProgress *p = d->pr;
    if (!p) return 0;
    if (dltotal > 0) p->total = dltotal;
    p->done = dlnow;
    double t = now_s();
    if (t - d->t_last >= .5) {
        float inst = (float)((dlnow - d->b_last) / (t - d->t_last));
        p->speed = p->speed <= 0 ? inst : p->speed * .6f + inst * .4f;
        d->t_last = t;
        d->b_last = dlnow;
    }
    return p->cancel ? 1 : 0;
}

int net_download(const char *url, const char *path, NetProgress *pr) {
    if (!inited) {
        snprintf(err, sizeof(err), "rede não iniciada");
        return NET_OFFLINE;
    }
    Dl d;
    memset(&d, 0, sizeof(d));
    d.pr = pr;
    d.buf = malloc(WBUF);
    if (!d.buf) return NET_ERR;
    d.fd = sceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (d.fd < 0) {
        free(d.buf);
        snprintf(err, sizeof(err), "não deu para criar %s", path);
        return NET_DISK;
    }
    d.t0 = d.t_last = now_s();
    CURL *c = curl_easy_init();
    common(c, url);
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, file_cb);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, &d);
    curl_easy_setopt(c, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(c, CURLOPT_XFERINFOFUNCTION, prog_cb);
    curl_easy_setopt(c, CURLOPT_XFERINFODATA, &d);
    curl_easy_setopt(c, CURLOPT_BUFFERSIZE, 256 * 1024L);
    CURLcode r = curl_easy_perform(c);
    long code = 0;
    curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &code);
    curl_easy_cleanup(c);
    if (r == CURLE_OK) flush(&d);
    sceIoClose(d.fd);
    free(d.buf);
    if (r == CURLE_ABORTED_BY_CALLBACK) {
        sceIoRemove(path);
        return NET_CANCELED;
    }
    if (d.failed || r == CURLE_WRITE_ERROR) {
        sceIoRemove(path);
        snprintf(err, sizeof(err), "falha ao gravar no cartão (sem espaço?)");
        return NET_DISK;
    }
    if (r != CURLE_OK) {
        sceIoRemove(path);
        if (code >= 400) {
            snprintf(err, sizeof(err), "HTTP %ld", code);
            return NET_HTTP;
        }
        snprintf(err, sizeof(err), "%s", curl_easy_strerror(r));
        return NET_ERR;
    }
    if (pr) pr->done = pr->total > 0 ? pr->total : pr->done;
    return NET_OK;
}
