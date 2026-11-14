/* store.json (gerado pelo site, tools/store.py) -> Post[] */
#include "catalog.h"
#include "fsutil.h"
#include "json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *S(const JsonValue *v, const char *key) { return dup_latin1(json_string(json_object_get(v, key), "")); }

static int cat_of(const char *c) {
    if (!strcmp(c, "ports")) return CAT_PORTS;
    if (!strcmp(c, "ferramentas")) return CAT_TOOLS;
    if (!strcmp(c, "traducoes")) return CAT_TRANS;
    return CAT_REVIEWS;
}

static void texts(const JsonValue *o, PostText *t) {
    memset(t, 0, sizeof(*t));
    t->title = S(o, "titulo");
    t->tag = S(o, "tag");
    t->status = S(o, "status");
    t->desc = S(o, "resumo");
    t->feature = S(o, "chamada");
    if (!t->feature[0]) t->feature = t->desc;
    t->bars_title = S(o, "titulo_barras");
    if (!t->bars_title[0]) t->bars_title = NULL;
    t->about = S(o, "sobre");
    const JsonValue *b = json_object_get(o, "blocos");
    for (int i = 0; i < json_array_count(b) && t->nblocks < MAX_BLOCKS; i++) {
        const JsonValue *e = json_array_get(b, i);
        const char *k = json_string(json_array_get(e, 0), "para");
        Block *x = &t->blocks[t->nblocks++];
        x->type = !strcmp(k, "sec") ? BLK_SEC : !strcmp(k, "item") ? BLK_ITEM : BLK_PARA;
        x->text = dup_latin1(json_string(json_array_get(e, 1), ""));
    }
    const JsonValue *bars = json_object_get(o, "barras");
    for (int i = 0; i < json_array_count(bars) && t->nbars < MAX_PAIRS; i++) {
        const JsonValue *e = json_array_get(bars, i);
        Pair *p = &t->bars[t->nbars++];
        p->a = dup_latin1(json_string(json_array_get(e, 0), ""));
        p->pct = (int)json_number(json_array_get(e, 1), 0);
        p->b = "";
        p->c = dup_latin1(json_string(json_array_get(e, 2), ""));
    }
    const char *lists[4] = {"ficha", "links", "fontes", "clock"};
    Pair *dsts[4] = {t->info, t->links, t->srcs, t->clock};
    int *ns[4] = {&t->ninfo, &t->nlinks, &t->nsrcs, &t->nclock};
    for (int l = 0; l < 4; l++) {
        const JsonValue *arr = json_object_get(o, lists[l]);
        Pair *dst = dsts[l];
        int *n = ns[l];
        for (int i = 0; i < json_array_count(arr) && *n < MAX_PAIRS; i++) {
            const JsonValue *e = json_array_get(arr, i);
            Pair *p = &dst[(*n)++];
            p->a = dup_latin1(json_string(json_array_get(e, 0), ""));
            p->b = dup_latin1(json_string(json_array_get(e, 1), ""));
            p->c = "";
        }
    }
}

int store_parse(const char *text, Post *out, int max, AppRelease *app) {
    JsonValue *root = json_parse(text);
    if (!root) return -1;
    if (app) {
        memset(app, 0, sizeof(*app));
        const JsonValue *a = json_object_get(root, "app");
        if (a && json_type(a) == JSON_OBJECT) {
            snprintf(app->version, sizeof(app->version), "%s", json_string(json_object_get(a, "versao"), ""));
            snprintf(app->url, sizeof(app->url), "%s", json_string(json_object_get(a, "url"), ""));
            utf8_to_latin1(json_string(json_object_get(a, "notas"), ""), app->notes, sizeof(app->notes));
            app->size = (long long)json_number(json_object_get(a, "tamanho"), 0);
        }
    }
    const JsonValue *posts = json_object_get(root, "posts");
    int n = 0;
    for (int i = 0; i < json_array_count(posts) && n < max; i++) {
        const JsonValue *o = json_array_get(posts, i);
        if (json_type(o) != JSON_OBJECT) continue;
        Post *p = &out[n++];
        memset(p, 0, sizeof(*p));
        snprintf(p->slug, sizeof(p->slug), "%s", json_string(json_object_get(o, "slug"), "post"));
        p->cat = cat_of(json_string(json_object_get(o, "cat"), ""));
        p->group = (int)json_number(json_object_get(o, "grupo"), 0);
        if (p->group < 0 || p->group >= GRP_COUNT) p->group = 0;
        snprintf(p->date, sizeof(p->date), "%s", json_string(json_object_get(o, "data"), ""));
        p->feat = json_bool(json_object_get(o, "destaque"), 0);
        p->pixel = json_bool(json_object_get(o, "pixel"), 0);
        p->progress = (int)json_number(json_object_get(o, "progresso"), -1);
        const char *video = json_string(json_object_get(o, "video"), "");
        p->video = video[0] ? dup_latin1(video) : NULL;
        const char *vf = json_string(json_object_get(o, "video_arquivo"), "");
        p->video_file = vf[0] ? dup_latin1(vf) : NULL;
        p->trophies = json_bool(json_object_get(o, "trofeus"), 0);
        texts(json_object_get(o, "pt"), &p->t[LANG_PT]);
        texts(json_object_get(o, "en"), &p->t[LANG_EN]);
        const char *capa = json_string(json_object_get(o, "capa"), "");
        if (capa[0]) {
            p->cover_url = dup_latin1(capa);
            p->cover = 1;
        }
        const JsonValue *caps = json_object_get(o, "capturas");
        for (int k = 0; k < json_array_count(caps) && p->shots < MAX_SHOTS; k++)
            p->shot_url[p->shots++] = dup_latin1(json_string(json_array_get(caps, k), ""));
        const JsonValue *dls = json_object_get(o, "downloads");
        for (int k = 0; k < json_array_count(dls) && p->ndls < MAX_DLS; k++) {
            const JsonValue *d = json_array_get(dls, k);
            Download *x = &p->dls[p->ndls++];
            const char *tp = json_string(json_object_get(d, "tipo"), "vpk");
            x->type = !strcmp(tp, "traducao") ? DL_TRANS : !strcmp(tp, "plugin") ? DL_PLUGIN : !strcmp(tp, "arquivo") ? DL_FILE : !strcmp(tp, "dados") ? DL_DATA : DL_VPK;
            x->name = S(d, "nome");
            x->url = S(d, "url");
            x->version = S(d, "versao");
            x->file = S(d, "arquivo");
            x->instr = S(d, "instrucoes");
            x->plugin = S(d, "plugin");
            x->size = (long long)json_number(json_object_get(d, "tamanho"), 0);
            x->needs_data = json_bool(json_object_get(d, "precisa_dados"), 0);
            const JsonValue *t = json_object_get(d, "titleids");
            for (int j = 0; j < json_array_count(t) && x->ntids < 8; j++)
                snprintf(x->tids[x->ntids++], 10, "%s", json_string(json_array_get(t, j), ""));
        }
    }
    json_free(root);
    return n;
}

void store_cache_path(const char *rel, char *out, int n) {
    /* "store/pizza/capa-1a2b3c4d.png" -> ux0:data/WolffsStore/cache/pizza/capa-1a2b3c4d.png */
    const char *r = rel;
    if (!strncmp(r, "store/", 6)) r += 6;
    snprintf(out, n, WS_CACHE "/%s", r);
}
