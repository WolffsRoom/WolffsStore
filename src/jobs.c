#include "jobs.h"
#include "fsutil.h"
#include "install.h"

#include <psp2/io/fcntl.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/shellutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Job jobs[MAX_JOBS]; /* posições fixas: o trabalhador guarda ponteiros para elas */
static int next_id = 1;
static volatile unsigned version;
static SceUID mtx = -1;

static void lock(void) { sceKernelLockMutex(mtx, 1, NULL); }
static void unlock(void) { sceKernelUnlockMutex(mtx, 1); }
static void set_state(Job *j, int s) {
    j->state = s;
    version++;
}

unsigned jobs_version(void) { return version; }

int jobs_count(void) {
    int n = 0;
    for (int i = 0; i < MAX_JOBS; i++) n += jobs[i].used;
    return n;
}

/* i-ésimo trabalho por ordem de criação */
Job *jobs_get(int i) {
    int last = 0;
    for (int k = 0; k <= i; k++) {
        Job *best = NULL;
        for (int s = 0; s < MAX_JOBS; s++)
            if (jobs[s].used && jobs[s].id > last && (!best || jobs[s].id < best->id)) best = &jobs[s];
        if (!best) return NULL;
        if (k == i) return best;
        last = best->id;
    }
    return NULL;
}

Job *jobs_find(const char *slug, int dl) {
    Job *best = NULL;
    for (int i = 0; i < MAX_JOBS; i++)
        if (jobs[i].used && jobs[i].kind != JK_SELF && jobs[i].dl == dl && !strcmp(jobs[i].slug, slug) &&
            (!best || jobs[i].id > best->id))
            best = &jobs[i];
    return best;
}

int jobs_active(void) {
    int n = 0;
    for (int i = 0; i < MAX_JOBS; i++)
        if (jobs[i].used && jobs[i].state < J_DONE) n++;
    return n;
}

void jobs_cancel(int id) {
    for (int i = 0; i < MAX_JOBS; i++)
        if (jobs[i].used && jobs[i].id == id && jobs[i].state < J_DONE) {
            jobs[i].pr.cancel = 1;
            if (jobs[i].state == J_QUEUED) set_state(&jobs[i], J_CANCELED);
        }
}

void jobs_clear_finished(void) {
    lock();
    for (int i = 0; i < MAX_JOBS; i++)
        if (jobs[i].used && jobs[i].state >= J_DONE) jobs[i].used = 0;
    version++;
    unlock();
}

/* posição livre; se não houver, reaproveita o concluído mais antigo */
static Job *slot(void) {
    Job *j = NULL;
    for (int i = 0; i < MAX_JOBS && !j; i++)
        if (!jobs[i].used) j = &jobs[i];
    if (!j)
        for (int i = 0; i < MAX_JOBS; i++)
            if (jobs[i].state >= J_DONE && (!j || jobs[i].id < j->id)) j = &jobs[i];
    if (!j) return NULL;
    memset(j, 0, sizeof(*j));
    j->used = 1;
    j->id = next_id++;
    return j;
}

int jobs_add(const Post *p, int dl, const char *title) {
    if (dl < 0 || dl >= p->ndls) return -1;
    const Download *d = &p->dls[dl];
    Job *old = jobs_find(p->slug, dl);
    if (old && old->state < J_DONE) return old->id; /* já está na fila */
    lock();
    Job *j = slot();
    if (!j) {
        unlock();
        return -1;
    }
    j->kind = d->type == DL_TRANS ? JK_TRANS : d->type == DL_PLUGIN ? JK_PLUGIN : d->type == DL_FILE ? JK_FILE : d->type == DL_DATA ? JK_DATA : JK_VPK;
    snprintf(j->name, sizeof(j->name), "%s", title);
    snprintf(j->url, sizeof(j->url), "%s", d->url);
    snprintf(j->file, sizeof(j->file), "%s", d->file[0] ? d->file : "download.bin");
    snprintf(j->version, sizeof(j->version), "%s", d->version);
    snprintf(j->slug, sizeof(j->slug), "%s", p->slug);
    j->dl = dl;
    for (int i = 0; i < d->ntids; i++) snprintf(j->tids[i], 10, "%s", d->tids[i]);
    j->ntids = d->ntids;
    j->size_hint = d->size;
    j->pr.total = d->size;
    set_state(j, J_QUEUED);
    unlock();
    return j->id;
}

int jobs_add_self(const AppRelease *app) {
    lock();
    Job *j = slot();
    if (!j) {
        unlock();
        return -1;
    }
    j->kind = JK_SELF;
    snprintf(j->name, sizeof(j->name), "Wolff's Store v%s", app->version);
    snprintf(j->url, sizeof(j->url), "%s", app->url);
    snprintf(j->file, sizeof(j->file), "WolffsStore-update.vpk");
    snprintf(j->version, sizeof(j->version), "%s", app->version);
    j->size_hint = j->pr.total = app->size;
    j->dl = -1;
    set_state(j, J_QUEUED);
    unlock();
    return j->id;
}

/* ------------------------------------------------------------------ trabalhador */

static void fail(Job *j, const char *why) {
    if (!j->pr.cancel) ws_log("download/instalação falhou: %s [%s] %s", j->name, j->url, why);
    utf8_to_latin1(why, j->msg, sizeof(j->msg));
    set_state(j, j->pr.cancel ? J_CANCELED : J_ERROR);
}

static int check_space(Job *j, double factor) {
    long long free_b = fs_free_space("ux0:");
    if (j->size_hint <= 0 || free_b < 0) return 1;
    long long need = (long long)(j->size_hint * factor) + 16 * 1024 * 1024;
    if (free_b >= need) return 1;
    char m[160];
    snprintf(m, sizeof(m), "Espaço insuficiente: precisa de %lld MB livres em ux0: (há %lld MB).",
             need / (1024 * 1024), free_b / (1024 * 1024));
    fail(j, m);
    return 0;
}

static int download(Job *j, const char *dst) {
    set_state(j, J_DOWNLOAD);
    char part[300];
    snprintf(part, sizeof(part), "%s.part", dst);
    int r = net_download(j->url, part, &j->pr);
    if (r != NET_OK) {
        fail(j, r == NET_CANCELED ? "Cancelado." : net_error());
        return -1;
    }
    if (j->kind == JK_VPK || j->kind == JK_SELF || j->kind == JK_TRANS || j->kind == JK_DATA) {
        /* zip/VPK começa com "PK"; uma página HTML (ex.: aviso do Google Drive) não */
        unsigned char m[2] = {0, 0};
        SceUID fd = sceIoOpen(part, SCE_O_RDONLY, 0);
        if (fd >= 0) {
            sceIoRead(fd, m, 2);
            sceIoClose(fd);
        }
        if (m[0] != 'P' || m[1] != 'K') {
            sceIoRemove(part);
            fail(j, m[0] == '<' ? "O link abriu uma página em vez do arquivo (Google Drive pedindo confirmação ou link sem acesso público)."
                               : "O arquivo baixado não é um zip/VPK válido.");
            return -1;
        }
    }
    sceIoRemove(dst);
    sceIoRename(part, dst);
    return 0;
}

static void run(Job *j) {
    char path[300];
    fs_mkdir_p(WS_DL);
    switch (j->kind) {
    case JK_VPK:
    case JK_SELF: {
        if (!check_space(j, 2.2)) return;
        snprintf(path, sizeof(path), WS_DL "/%s", j->kind == JK_SELF ? "self.vpk" : "app.vpk");
        if (download(j, path) < 0) return;
        if (j->kind == JK_SELF) {
            /* só prepara; quem instala é o updater, com o app fechado */
            set_state(j, J_EXTRACT);
            fs_rm_rf(WS_PKG_SELF);
            fs_mkdir_p(WS_PKG_SELF);
            int r = inst_extract(path, WS_PKG_SELF, NULL, 0, &j->pr);
            if (r == 0) r = inst_make_head_bin(WS_PKG_SELF);
            sceIoRemove(path);
            if (r < 0) {
                fail(j, inst_error());
                return;
            }
            snprintf(j->msg, sizeof(j->msg), "pronto");
            set_state(j, J_DONE);
            return;
        }
        set_state(j, J_EXTRACT);
        sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN); /* não sair no meio da instalação */
        j->pr.done = 0;
        int r = inst_install_vpk(path, &j->pr, &j->phase); /* phase: 0 extraindo, 1 instalando */
        sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN);
        sceIoRemove(path);
        if (r < 0) {
            fail(j, inst_error());
            return;
        }
        snprintf(j->msg, sizeof(j->msg), "ux0:app/%s", inst_last_titleid());
        set_state(j, J_DONE);
        return;
    }
    case JK_TRANS: {
        if (!check_space(j, 3.0)) return;
        snprintf(path, sizeof(path), WS_DL "/traducao.zip");
        if (download(j, path) < 0) return;
        set_state(j, J_EXTRACT);
        for (int i = 0; i < j->ntids; i++) {
            char dst[64], a[40], b[40];
            snprintf(dst, sizeof(dst), "ux0:rePatch/%s", j->tids[i]);
            snprintf(a, sizeof(a), "rePatch/%s/", j->tids[i]);
            snprintf(b, sizeof(b), "%s/", j->tids[i]);
            const char *strip[3] = {a, b, "rePatch/"};
            fs_mkdir_p(dst);
            if (inst_extract(path, dst, strip, 3, &j->pr) < 0) {
                sceIoRemove(path);
                fail(j, j->pr.cancel ? "Cancelado." : inst_error());
                return;
            }
        }
        sceIoRemove(path);
        snprintf(j->msg, sizeof(j->msg), "ux0:rePatch/%s%s", j->tids[0], j->ntids > 1 ? " (+)" : "");
        set_state(j, J_DONE);
        return;
    }
    case JK_DATA: { /* dados do jogo: zip extraído em ux0:data/ */
        if (!check_space(j, 3.0)) return;
        snprintf(path, sizeof(path), WS_DL "/dados.zip");
        if (download(j, path) < 0) return;
        set_state(j, J_EXTRACT);
        const char *strip[2] = {"ux0/data/", "data/"};
        if (inst_extract(path, "ux0:data", strip, 2, &j->pr) < 0) {
            sceIoRemove(path);
            fail(j, j->pr.cancel ? "Cancelado." : inst_error());
            return;
        }
        sceIoRemove(path);
        snprintf(j->msg, sizeof(j->msg), "ux0:data/");
        set_state(j, J_DONE);
        return;
    }
    default: { /* plugin / arquivo */
        if (!check_space(j, 1.05)) return;
        fs_mkdir_p("ux0:Downloads");
        snprintf(path, sizeof(path), "ux0:Downloads/%s", j->file);
        if (download(j, path) < 0) return;
        snprintf(j->msg, sizeof(j->msg), "%s", path);
        set_state(j, J_DONE);
        return;
    }
    }
}

static int worker(SceSize args, void *argp) {
    (void)args;
    (void)argp;
    for (;;) {
        Job *j = NULL;
        lock();
        for (int i = 0; i < MAX_JOBS; i++)
            if (jobs[i].used && jobs[i].state == J_QUEUED && (!j || jobs[i].id < j->id)) j = &jobs[i];
        unlock();
        if (!j) {
            sceKernelDelayThread(150 * 1000);
            continue;
        }
        net_init();
        if (!net_online(5000)) {
            fail(j, "Sem conexão com a internet.");
            continue;
        }
        sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
        run(j);
    }
    return 0;
}

/* mantém o Vita acordado enquanto houver download */
static int ticker(SceSize args, void *argp) {
    (void)args;
    (void)argp;
    for (;;) {
        if (jobs_active()) sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
        sceKernelDelayThread(1000 * 1000);
    }
    return 0;
}

void jobs_init(void) {
    mtx = sceKernelCreateMutex("ws_jobs", 0, 0, NULL);
    sceShellUtilInitEvents(0);
    fs_rm_rf(WS_DL); /* sobras de downloads interrompidos */
    SceUID t = sceKernelCreateThread("ws_jobs", worker, 0x10000100 + 20, 0x20000, 0, 0, NULL);
    if (t >= 0) sceKernelStartThread(t, 0, NULL);
    t = sceKernelCreateThread("ws_tick", ticker, 0x10000100 + 30, 0x1000, 0, 0, NULL);
    if (t >= 0) sceKernelStartThread(t, 0, NULL);
}
