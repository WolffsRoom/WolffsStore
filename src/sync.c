#include "sync.h"
#include "fsutil.h"
#include "net.h"
#include "json.h"

#include <psp2/io/fcntl.h>
#include <psp2/kernel/threadmgr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static volatile int state = SYNC_IDLE;
static volatile float progress;
static Post staged[MAX_POSTS];
static int nstaged;
static AppRelease staged_app;

/* A partir da v0.42, a release do próprio repositório é a fonte primária da
 * atualização. O campo app do store.json continua como fallback para levar a
 * v0.41 até esta versão. */
static void fetch_latest_release(AppRelease *app) {
    int size = 0;
    char *text = net_fetch(RELEASE_API, &size);
    if (!text) {
        ws_log("update: falhou ao consultar release: %s", net_error());
        return;
    }
    JsonValue *root = json_parse(text);
    free(text);
    if (!root || json_type(root) != JSON_OBJECT) {
        if (root) json_free(root);
        ws_log("update: resposta de release inválida (%d bytes)", size);
        return;
    }
    const char *tag = json_string(json_object_get(root, "tag_name"), "");
    const char *body = json_string(json_object_get(root, "body"), "");
    const JsonValue *assets = json_object_get(root, "assets");
    const JsonValue *asset = NULL;
    for (int i = 0; i < json_array_count(assets); i++) {
        const JsonValue *candidate = json_array_get(assets, i);
        if (!strcmp(json_string(json_object_get(candidate, "name"), ""), RELEASE_ASSET)) {
            asset = candidate;
            break;
        }
    }
    const char *url = asset ? json_string(json_object_get(asset, "browser_download_url"), "") : "";
    if (tag[0] && url[0]) {
        snprintf(app->version, sizeof(app->version), "%s", tag[0] == 'v' || tag[0] == 'V' ? tag + 1 : tag);
        snprintf(app->url, sizeof(app->url), "%s", url);
        utf8_to_latin1(body, app->notes, sizeof(app->notes));
        app->size = (long long)json_number(json_object_get(asset, "size"), 0);
    } else {
        ws_log("update: release sem o asset %s", RELEASE_ASSET);
    }
    json_free(root);
}

int sync_state(void) { return state; }
float sync_progress(void) { return progress; }

int store_load_cache(Post *dst, int max, AppRelease *app) {
    char *t = fs_read(STORE_CACHE_JSON, NULL);
    if (!t) return -1;
    int n = store_parse(t, dst, max, app);
    free(t);
    return n;
}

/* baixa a imagem se ainda não estiver no cache (o nome tem o hash, então nunca fica velha) */
static void fetch_image(const char *rel) {
    if (!rel || !rel[0]) return;
    char path[256];
    store_cache_path(rel, path, sizeof(path));
    if (fs_exists(path)) return;
    char dir[256];
    snprintf(dir, sizeof(dir), "%s", path);
    char *s = strrchr(dir, '/');
    if (s) *s = 0;
    fs_mkdir_p(dir);
    char url[512], tmp[272];
    snprintf(url, sizeof(url), STORE_BASE "%s", rel);
    snprintf(tmp, sizeof(tmp), "%s.part", path);
    if (net_download(url, tmp, NULL) == NET_OK) sceIoRename(tmp, path);
}

static int sync_thread(SceSize args, void *argp) {
    (void)args;
    (void)argp;
    net_init();
    if (!net_online(8000)) {
        state = SYNC_OFFLINE;
        return sceKernelExitDeleteThread(0);
    }
    int size = 0;
    char *t = net_fetch(STORE_JSON, &size);
    if (!t) {
        ws_log("sync: falhou ao baixar %s: %s", STORE_JSON, net_error());
        state = SYNC_FAIL;
        return sceKernelExitDeleteThread(0);
    }
    int n = store_parse(t, staged, MAX_POSTS, &staged_app);
    if (n <= 0) {
        free(t);
        ws_log("sync: store.json inválido (%d bytes)", size);
        state = SYNC_FAIL;
        return sceKernelExitDeleteThread(0);
    }
    fetch_latest_release(&staged_app);
    /* capas primeiro (aparecem nas listas), depois as capturas */
    int total = 0, done = 0;
    for (int i = 0; i < n; i++) total += 1 + staged[i].shots;
    for (int i = 0; i < n; i++) {
        fetch_image(staged[i].cover_url);
        progress = (float)++done / total;
    }
    for (int i = 0; i < n; i++)
        for (int k = 0; k < staged[i].shots; k++) {
            fetch_image(staged[i].shot_url[k]);
            progress = (float)++done / total;
        }
    fs_mkdir_p(WS_CACHE);
    fs_write(STORE_CACHE_JSON, t, size);
    free(t);
    nstaged = n;
    state = SYNC_READY;
    return sceKernelExitDeleteThread(0);
}

void sync_start(void) {
    if (state == SYNC_RUNNING) return;
    state = SYNC_RUNNING;
    progress = 0;
    SceUID th = sceKernelCreateThread("ws_sync", sync_thread, 0x10000100 + 10, 0x10000, 0, 0, NULL);
    if (th < 0 || sceKernelStartThread(th, 0, NULL) < 0) state = SYNC_FAIL;
}

int sync_take(Post *dst, int max, AppRelease *app) {
    if (state != SYNC_READY) return -1;
    int n = nstaged < max ? nstaged : max;
    memcpy(dst, staged, sizeof(Post) * n);
    if (app) *app = staged_app;
    state = SYNC_IDLE;
    return n;
}
