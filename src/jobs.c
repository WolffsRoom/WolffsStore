#include "jobs.h"
#include "fsutil.h"
#include "install.h"
#include "touch_plugin.h"

#include <psp2/io/fcntl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
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

int jobs_add(const Post *p, int dl, const char *title, int confirm_replace) {
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
    if(d->type==DL_TAI_PLUGIN &&
       (strcmp(p->slug,"dd-touchonvita") || strcmp(d->file,"DDTouchUI-v0.59.zip"))) {
        unlock();return -1;
    }
    j->kind = d->type == DL_TAI_PLUGIN ? JK_TAI_PLUGIN : d->type == DL_TRANS ? JK_TRANS : d->type == DL_APP_FILES ? JK_APP_FILES : d->type == DL_PLUGIN ? JK_PLUGIN : d->type == DL_FILE ? JK_FILE : d->type == DL_DATA ? JK_DATA : JK_VPK;
    snprintf(j->name, sizeof(j->name), "%s", title);
    snprintf(j->url, sizeof(j->url), "%s", d->url);
    j->nmirrors = d->nmirrors;
    for (int k = 0; k < j->nmirrors; k++)
        snprintf(j->mirrors[k], sizeof(j->mirrors[k]), "%s", d->mirrors[k]);
    /* All Wolff's Room protected translation archives use one password.
     * Never rely on the downloaded catalog to provide it correctly. */
    const char *ext = strrchr(d->file, '.');
    int protected_translation = d->type == DL_TRANS || d->type == DL_APP_FILES ||
        (d->type == DL_VPK && ext &&
         (!strcasecmp(ext, ".zip") || !strcasecmp(ext, ".rar")));
    snprintf(j->password, sizeof(j->password), "%s",
             protected_translation ? "wolffsroom" : (d->password ? d->password : ""));
    j->confirm_replace = confirm_replace && (d->type == DL_TRANS || d->type == DL_APP_FILES);
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

/* Resolve o botão do MediaFire sem depender de links temporários. */
static int resolve_mediafire(const char *page_url, char *out, size_t cap) {
    int size = 0;
    /* The public MediaFire HTML can stall indefinitely on Vita TLS/CDN.
     * Give up after 15s so another mirror can be attempted. */
    char *html = net_fetch_timeout(page_url, &size, 15L);
    if (!html) return -1;
    char *btn = strstr(html, "id=\"downloadButton\"");
    char *href = NULL, *scan = html;
    while (btn && (scan = strstr(scan, "href=\"")) && scan < btn) {
        href = scan + 6;
        scan += 6;
    }
    int result = -1;
    if (href && !strncmp(href, "https://download", 16)) {
        const char *end = strchr(href, '"');
        if (end && end > href && (size_t)(end - href) < cap) {
            memcpy(out, href, (size_t)(end - href));
            out[end - href] = 0;
            if (strstr(out, ".mediafire.com/")) result = 0;
        }
    }
    free(html);
    return result;
}

static int download(Job *j, const char *dst) {
    set_state(j, J_RESOLVE);
    char part[300];
    snprintf(part, sizeof(part), "%s.part", dst);
    char original[512];
    snprintf(original, sizeof(original), "%s", j->url);
    char error[192] = "";
    for (int attempt = 0; attempt <= j->nmirrors; attempt++) {
        if (j->pr.cancel) { fail(j, "Cancelado."); return -1; }
        const char *candidate = attempt ? j->mirrors[attempt - 1] : original;
        if (!candidate[0]) continue;
        char resolved[640];
        snprintf(resolved, sizeof(resolved), "%s", candidate);
        if (strstr(candidate, "mega.nz/file/")) {
            /* Mega usa chave de criptografia e API própria: não é URL de arquivo direto. */
            ws_log("download: espelho MEGA não suporta acesso direto no Vita: %s", candidate);
            snprintf(error, sizeof(error), "MEGA requer download manual no navegador.");
            continue;
        }
        if (strstr(candidate, "mediafire.com/file/")) {
            set_state(j, J_RESOLVE);
            if (resolve_mediafire(candidate, resolved, sizeof(resolved)) < 0) {
                ws_log("download: MediaFire não forneceu URL direta");
                snprintf(error, sizeof(error), "Não foi possível resolver o link MediaFire.");
                continue;
            }
        }
        snprintf(j->url, sizeof(j->url), "%s", resolved);
        ws_log("download: attempting mirror %d/%d [%s]", attempt + 1,
               j->nmirrors + 1, strstr(candidate, "drive.usercontent.google.com")
               ? "Google Drive" : strstr(candidate, "mediafire.com") ? "MediaFire" : "external");
        set_state(j, J_DOWNLOAD);
        j->pr.done = 0;
        j->pr.total = j->size_hint;
        j->pr.speed = 0;
        sceIoRemove(part);
        int r = net_download(resolved, part, &j->pr);
        if (r != NET_OK) {
            snprintf(error, sizeof(error), "%s", r == NET_CANCELED ? "Cancelado." : net_error());
            ws_log("download: espelho %d falhou: %s", attempt + 1, error);
            set_state(j, J_RESOLVE);
            sceIoRemove(part);
            if (r == NET_CANCELED || j->pr.cancel) { fail(j, "Cancelado."); return -1; }
            continue;
        }
        if (j->kind == JK_VPK || j->kind == JK_SELF || j->kind == JK_TRANS ||
            j->kind == JK_DATA || j->kind == JK_APP_FILES) {
            unsigned char magic[8] = {0};
            SceUID fd = sceIoOpen(part, SCE_O_RDONLY, 0);
            if (fd >= 0) { sceIoRead(fd, magic, sizeof(magic)); sceIoClose(fd); }
            int is_zip = magic[0] == 'P' && magic[1] == 'K';
            int is_rar = !memcmp(magic, "Rar!\x1A\x07", 6);
            if (!is_zip && !((j->kind == JK_APP_FILES || j->kind == JK_TRANS) && is_rar)) {
                snprintf(error, sizeof(error), "O espelho não retornou um ZIP/VPK ou RAR válido.");
                ws_log("download: espelho %d retornou dados inválidos", attempt + 1);
                sceIoRemove(part);
                continue;
            }
        }
        sceIoRemove(dst);
        if (sceIoRename(part, dst) < 0) {
            fail(j, "Não foi possível gravar o download.");
            return -1;
        }
        return 0;
    }
    fail(j, error[0] ? error : "Nenhum espelho disponível.");
    return -1;
}

/* Busca um VPK dentro do ZIP: até 4 níveis, sem instalar arquivos estranhos. */
static int find_nested_vpk(const char *dir, char *out, size_t cap, int depth) {
    if (depth > 4) return 0;
    SceUID d = sceIoDopen(dir);
    if (d < 0) return 0;
    int found = 0;
    SceIoDirent e;
    while (!found) {
        memset(&e, 0, sizeof(e));
        if (sceIoDread(d, &e) <= 0) break;
        if (!strcmp(e.d_name, ".") || !strcmp(e.d_name, "..")) continue;
        char path[512];
        if (snprintf(path, sizeof(path), "%s/%s", dir, e.d_name) >= (int)sizeof(path)) continue;
        if (SCE_S_ISDIR(e.d_stat.st_mode)) {
            found = find_nested_vpk(path, out, cap, depth + 1);
        } else if (SCE_S_ISREG(e.d_stat.st_mode)) {
            const char *ext = strrchr(e.d_name, '.');
            if (ext && !strcasecmp(ext, ".vpk") && strlen(path) < cap) {
                snprintf(out, cap, "%s", path);
                found = 1;
            }
        }
    }
    sceIoDclose(d);
    return found;
}

/* Arquivos de transporte não pertencem à pasta ux0:app do jogo. */
static int app_skip_aux(const char *name) {
    const char *ext = strrchr(name, '.');
    return !strcasecmp(name, ".DS_Store") ||
           (ext && (!strcasecmp(ext, ".torrent") ||
                    !strcasecmp(ext, ".url") || !strcasecmp(ext, ".nfo") ||
                    !strcasecmp(ext, ".md") || !strcasecmp(ext, ".txt")));
}

/* Pré-valida a pasta PCSF00007 antes de tocar no jogo instalado.
 * Um RAR que só contém um .torrent não é um pacote instalável. */
static int app_payload_valid(const char *dir, int depth, int *files) {
    if (depth > 12) return -1;
    SceUID d = sceIoDopen(dir);
    if (d < 0) return -1;
    SceIoDirent e;
    int result = 0;
    for (;;) {
        memset(&e, 0, sizeof(e));
        if (sceIoDread(d, &e) <= 0) break;
        if (!strcmp(e.d_name, ".") || !strcmp(e.d_name, "..")) continue;
        char path[512];
        if (snprintf(path, sizeof(path), "%s/%s", dir, e.d_name) >= (int)sizeof(path)) { result = -1; break; }
        if (SCE_S_ISDIR(e.d_stat.st_mode)) {
            if (app_payload_valid(path, depth + 1, files) < 0) { result = -1; break; }
        } else if (SCE_S_ISREG(e.d_stat.st_mode)) {
            if (app_skip_aux(e.d_name)) continue;
            if (e.d_stat.st_size > 0) (*files)++;
        }
    }
    sceIoDclose(d);
    return result;
}

/* Primeira passagem: identifica conflitos com backups já existentes antes de copiar. */
static int app_preflight(const char *src, const char *dst, const char *bak, int depth) {
    if (depth > 12) return -1;
    SceUID d = sceIoDopen(src);
    if (d < 0) return -1;
    SceIoDirent e;
    int result = 0;
    for (;;) {
        memset(&e, 0, sizeof(e));
        if (sceIoDread(d, &e) <= 0) break;
        if (!strcmp(e.d_name, ".") || !strcmp(e.d_name, "..")) continue;
        char a[512], b[512], c[512];
        if (snprintf(a, sizeof(a), "%s/%s", src, e.d_name) >= (int)sizeof(a) ||
            snprintf(b, sizeof(b), "%s/%s", dst, e.d_name) >= (int)sizeof(b) ||
            snprintf(c, sizeof(c), "%s/%s", bak, e.d_name) >= (int)sizeof(c)) { result = -1; break; }
        if (app_skip_aux(e.d_name) && SCE_S_ISREG(e.d_stat.st_mode)) continue;
        if (SCE_S_ISDIR(e.d_stat.st_mode)) {
            if (fs_exists(b) && !fs_is_dir(b)) { result = -1; break; }
            if (app_preflight(a, b, c, depth + 1) < 0) { result = -1; break; }
        } else if (SCE_S_ISREG(e.d_stat.st_mode)) {
            if (fs_is_dir(b) || (fs_exists(b) && fs_exists(c))) { result = -1; break; }
        } else { result = -1; break; }
    }
    sceIoDclose(d);
    return result;
}

/* Mescla arquivos com cópia de segurança dos originais.
 * Cada arquivo novo é gravado antes em .wsnew, nunca truncando o original. */
static int app_apply_files(const char *src, const char *dst, const char *bak,
                           int depth, volatile int *cancel) {
    if (depth > 12) return -1;
    SceUID d = sceIoDopen(src);
    if (d < 0) return -1;
    SceIoDirent e;
    int result = 0;
    for (;;) {
        memset(&e, 0, sizeof(e));
        if (sceIoDread(d, &e) <= 0) break;
        if (cancel && *cancel) { result = -2; break; }
        if (!strcmp(e.d_name, ".") || !strcmp(e.d_name, "..")) continue;
        char a[512], b[512], c[512], temp[512];
        if (snprintf(a, sizeof(a), "%s/%s", src, e.d_name) >= (int)sizeof(a) ||
            snprintf(b, sizeof(b), "%s/%s", dst, e.d_name) >= (int)sizeof(b) ||
            snprintf(c, sizeof(c), "%s/%s", bak, e.d_name) >= (int)sizeof(c) ||
            snprintf(temp, sizeof(temp), "%s.wsnew", b) >= (int)sizeof(temp)) { result = -1; break; }
        if (app_skip_aux(e.d_name) && SCE_S_ISREG(e.d_stat.st_mode)) continue;
        if (SCE_S_ISDIR(e.d_stat.st_mode)) {
            fs_mkdir_p(b);
            if (app_apply_files(a, b, c, depth + 1, cancel) < 0) { result = -1; break; }
        } else if (SCE_S_ISREG(e.d_stat.st_mode)) {
            if (fs_exists(temp)) { result = -1; break; }
            if (fs_copy(a, temp) < 0) { sceIoRemove(temp); result = -1; break; }
            int existed = fs_exists(b);
            if (existed) {
                fs_mkdir_p(bak);
                if (fs_copy(b, c) < 0 || sceIoRemove(b) < 0) {
                    sceIoRemove(temp);
                    result = -1;
                    break;
                }
            }
            if (sceIoRename(temp, b) < 0) {
                if (existed && fs_copy(c, b) < 0)
                    ws_log("WipeOut: falha ao restaurar %s de %s", b, c);
                sceIoRemove(temp);
                result = -1;
                break;
            }
        } else { result = -1; break; }
    }
    sceIoDclose(d);
    return result;
}

static void run(Job *j) {
    char path[300];
    fs_mkdir_p(WS_DL);
    switch (j->kind) {
    case JK_VPK:
    case JK_SELF: {
        const char *ext = strrchr(j->file, '.');
        int wrapped = j->kind == JK_VPK && ext &&
                      (!strcasecmp(ext, ".zip") || !strcasecmp(ext, ".rar"));
        if (!check_space(j, wrapped ? 4.5 : 2.2)) return;
        snprintf(path, sizeof(path), WS_DL "/%s",
                 j->kind == JK_SELF ? "self.vpk" : wrapped ? "wrapped-app.zip" : "app.vpk");
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
        char inner[512];
        const char *to_install = path;
        const char *stage = WS_DL "/wrapped-vpk";
        if (wrapped) {
            set_state(j, J_EXTRACT);
            fs_rm_rf(stage);
            fs_mkdir_p(stage);
            j->pr.done = 0;
            int r = inst_extract_password(path, stage, NULL, 0, j->password, &j->pr);
            if (r < 0 || !find_nested_vpk(stage, inner, sizeof(inner), 0)) {
                fs_rm_rf(stage);
                sceIoRemove(path);
                fail(j, r < 0 ? inst_error() : "O ZIP não contém um VPK instalável.");
                return;
            }
            to_install = inner;
        }
        set_state(j, J_EXTRACT);
        sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN); /* não sair no meio da instalação */
        j->pr.done = 0;
        int r = inst_install_vpk(to_install, &j->pr, &j->phase); /* promoter nativo do app */
        sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN);
        if (wrapped) fs_rm_rf(stage);
        sceIoRemove(path);
        if (r < 0) {
            fail(j, inst_error());
            return;
        }
        snprintf(j->msg, sizeof(j->msg), "ux0:app/%s", inst_last_titleid());
        set_state(j, J_DONE);
        return;
    }
    case JK_APP_FILES: {
        /* Arquivos de tradução do jogo base: nunca instala um torrent ou RAR vazio. */
        if (j->ntids != 1 || strcmp(j->tids[0], "PCSF00007")) {
            fail(j, "Pacote app_arquivos sem TitleID WipEout válido."); return;
        }
        if (!j->confirm_replace) {
            fail(j, "Confirme a instalação sobre os arquivos do jogo."); return;
        }
        const char *dst = "ux0:app/PCSF00007";
        const char *stage = WS_DL "/app-files-stage";
        const char *backup = "ux0:data/WolffsStore/backups/PCSF00007";
        if (!fs_is_dir(dst)) {
            fail(j, "Instale WipEout 2048 PCSF00007 antes da tradução."); return;
        }
        if (!check_space(j, 5.0)) return;
        snprintf(path, sizeof(path), WS_DL "/app-files.rar");
        if (download(j, path) < 0) return;
        set_state(j, J_EXTRACT);
        fs_rm_rf(stage);
        fs_mkdir_p(stage);
        const char *strip[] = { "CONSOLE/ux0/app/", "CONSOLE/app/", "ux0/app/", "app/" };
        j->pr.done = 0;
        int r = inst_extract_password(path, stage, strip, 4, j->password, &j->pr);
        char payload[256];
        snprintf(payload, sizeof(payload), "%s/%s", stage, j->tids[0]);
        int count = 0;
        if (r < 0 || !fs_is_dir(payload) ||
            app_payload_valid(payload, 0, &count) < 0 || !count) {
            fs_rm_rf(stage);
            sceIoRemove(path);
            fail(j, r < 0 ? inst_error() :
                 "Arquivo só tem torrent ou não contém PCSF00007. Obtenha o pacote completo.");
            return;
        }
        if (app_preflight(payload, dst, backup, 0) < 0) {
            fs_rm_rf(stage);
            sceIoRemove(path);
            fail(j, "Backup ou arquivo do jogo em conflito; instalação cancelada.");
            return;
        }
        set_state(j, J_INSTALL);
        sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN);
        r = app_apply_files(payload, dst, backup, 0, &j->pr.cancel);
        sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN);
        fs_rm_rf(stage);
        sceIoRemove(path);
        if (r < 0) {
            fail(j, "Falha na cópia; confira os backups em ux0:data/WolffsStore/backups.");
            return;
        }
        snprintf(j->msg, sizeof(j->msg), "%s", dst);
        set_state(j, J_DONE);
        return;
    }
    case JK_TRANS: {
        if (!check_space(j, 4.0)) return;
        if (j->ntids < 1) { fail(j, "Tradução sem TitleID."); return; }
        snprintf(path, sizeof(path), WS_DL "/traducao.zip");
        if (download(j, path) < 0) return;
        set_state(j, J_EXTRACT);
        fs_mkdir_p("ux0:rePatch");
        for (int i = 0; i < j->ntids; i++) {
            char dst[80], backup[96], stage[112], marker[32];
            snprintf(dst, sizeof(dst), "ux0:rePatch/%s", j->tids[i]);
            snprintf(backup, sizeof(backup), "%s_bkp", dst);
            snprintf(stage, sizeof(stage), "ux0:rePatch/.WolffsStore-%s.tmp", j->tids[i]);
            snprintf(marker, sizeof(marker), "@%s/", j->tids[i]);
            const char *strip[1] = {marker};
            if (fs_is_dir(dst) && !j->confirm_replace) {
                sceIoRemove(path);
                fail(j, "Já existe mod para este jogo; confirme o backup antes de instalar.");
                return;
            }
            if (fs_exists(dst) && fs_exists(backup)) {
                sceIoRemove(path);
                fail(j, "Backup _bkp já existe. Renomeie-o no VitaShell antes de substituir.");
                return;
            }
            /* Prepara fora da pasta ativa; falha de ZIP/senha não altera o mod instalado. */
            if (fs_exists(stage)) fs_rm_rf(stage);
            fs_mkdir_p(stage);
            j->pr.done = 0;
            int r = inst_extract_password(path, stage, strip, 1, j->password, &j->pr);
            int files = 0;
            if (r < 0 || app_payload_valid(stage, 0, &files) < 0 || !files) {
                fs_rm_rf(stage);
                sceIoRemove(path);
                fail(j, j->pr.cancel ? "Cancelado." :
                     (r < 0 ? inst_error() : "Pacote sem arquivos sob TitleID: tradução inválida."));
                return;
            }
            if (j->pr.cancel) {
                fs_rm_rf(stage);
                sceIoRemove(path);
                fail(j, "Cancelado.");
                return;
            }
            int had_old = fs_exists(dst);
            if (had_old && sceIoRename(dst, backup) < 0) {
                fs_rm_rf(stage);
                sceIoRemove(path);
                fail(j, "Não foi possível criar PCSE..._bkp; mod anterior preservado.");
                return;
            }
            if (sceIoRename(stage, dst) < 0) {
                if (had_old && sceIoRename(backup, dst) < 0)
                    ws_log("ERRO: falha ao restaurar mod %s de %s", dst, backup);
                fs_rm_rf(stage);
                sceIoRemove(path);
                fail(j, "Não foi possível ativar tradução; verifique o backup _bkp.");
                return;
            }
            ws_log("rePatch: %s instalado; backup existente=%d", dst, had_old);
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
    case JK_TAI_PLUGIN: {
        /* Download first; the confirmed, allowlisted installer is separate. */
        if(!check_space(j, 4.0))return;
        fs_mkdir_p("ux0:Downloads/Plugins");
        snprintf(path,sizeof(path),"ux0:Downloads/Plugins/%s",j->file);
        if(download(j,path)<0)return;
        if(j->pr.cancel){fail(j,"Cancelado.");return;}
        set_state(j,J_INSTALL);
        sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN);
        int result=touch_plugin_install(path,&j->pr);
        sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN);
        if(result<0){fail(j,touch_plugin_error());return;}
        snprintf(j->msg,sizeof(j->msg),"ur0:tai/DDTouchUI.suprx");
        set_state(j,J_DONE);
        return;
    }
    default: { /* plugin / arquivo: download only, no taiHEN changes */
        if (!check_space(j, 1.05)) return;
        fs_mkdir_p(j->kind==JK_PLUGIN ? "ux0:Downloads/Plugins" : "ux0:Downloads");
        snprintf(path, sizeof(path), "%s/%s",
                 j->kind==JK_PLUGIN ? "ux0:Downloads/Plugins" : "ux0:Downloads", j->file);
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
