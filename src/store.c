/* store.json (gerado pelo site, tools/store.py) -> Post[] */
#include "catalog.h"
#include "fsutil.h"
#include "json.h"
#include "polls.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *S(const JsonValue *v, const char *key) { return dup_latin1(json_string(json_object_get(v, key), "")); }

static int cat_of(const char *c) {
    if (!strcmp(c, "ports")) return CAT_PORTS;
    if (!strcmp(c, "ferramentas")) return CAT_TOOLS;
    if (!strcmp(c, "traducoes")) return CAT_TRANS;
    if (!strcmp(c, "journal") || !strcmp(c, "reviews")) return CAT_REVIEWS;
    return CAT_REVIEWS;
}
static int subcat_of(const char *c) {
    if (!strcmp(c, "apps")) return SUB_APP;
    if (!strcmp(c, "ferramentas")) return SUB_TOOL;
    if (!strcmp(c, "modificacoes")) return SUB_MOD;
    if (!strcmp(c, "plugins")) return SUB_PLUGIN;
    if (!strcmp(c, "news")) return SUB_NEWS;
    if (!strcmp(c, "enquetes")) return SUB_POLL;
    return SUB_REVIEW;
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
    poll_set_api(json_string(json_object_get(root,"enquetes_api"),""));
    const JsonValue *posts = json_object_get(root, "posts");
    int n = 0;
    for (int i = 0; i < json_array_count(posts) && n < max; i++) {
        const JsonValue *o = json_array_get(posts, i);
        if (json_type(o) != JSON_OBJECT) continue;
        Post *p = &out[n++];
        memset(p, 0, sizeof(*p));
        p->github_downloads_total = -1;
        const JsonValue *downloads_count=json_object_get(o,"github_downloads_total");
        if(downloads_count && json_type(downloads_count)==JSON_NUMBER) {
            double count=json_number(downloads_count,-1);
            if(count>=0 && count<9007199254740992.0)
                p->github_downloads_total=(long long)count;
        }
        snprintf(p->slug, sizeof(p->slug), "%s", json_string(json_object_get(o, "slug"), "post"));
        const char *category = json_string(json_object_get(o, "cat"), "");
        p->cat = cat_of(category);
        const char *subcategory = json_string(json_object_get(o, "subcategoria"), "");
        p->subcat = subcategory[0] ? subcat_of(subcategory) :
                    p->cat == CAT_TOOLS ? SUB_TOOL : SUB_REVIEW;
        if (p->cat == CAT_REVIEWS && p->subcat == SUB_POLL) {
            const JsonValue *poll=json_object_get(o,"enquete");
            if (poll && json_type(poll)==JSON_OBJECT) {
                snprintf(p->poll_id,sizeof(p->poll_id),"%s",json_string(json_object_get(poll,"id"),""));
                utf8_to_latin1(json_string(json_object_get(poll,"question"),""),
                               p->poll_question,sizeof(p->poll_question));
                snprintf(p->poll_starts_at,sizeof(p->poll_starts_at),"%s",
                         json_string(json_object_get(poll,"starts_at"),""));
                snprintf(p->poll_ends_at,sizeof(p->poll_ends_at),"%s",
                         json_string(json_object_get(poll,"ends_at"),""));
                const JsonValue *opts=json_object_get(poll,"options_pt");
                if(!opts)opts=json_object_get(poll,"options");
                const JsonValue *opts_en=json_object_get(poll,"options_en");
                const char *question_pt=json_string(json_object_get(poll,"question_pt"),"");
                const char *question_en=json_string(json_object_get(poll,"question_en"),"");
                if(question_pt[0])utf8_to_latin1(question_pt,p->poll_question,sizeof(p->poll_question));
                utf8_to_latin1(question_en[0]?question_en:
                                json_string(json_object_get(poll,"question"),""),
                                p->poll_question_en,sizeof(p->poll_question_en));
                for(int x=0;x<json_array_count(opts) && x<8;x++) {
                    const char *pt_opt=json_string(json_array_get(opts,x),"");
                    utf8_to_latin1(pt_opt,p->poll_options[x],sizeof(p->poll_options[x]));
                    const char *en_opt=json_string(json_array_get(opts_en,x),"");
                    utf8_to_latin1(en_opt[0]?en_opt:pt_opt,
                                    p->poll_options_en[x],sizeof(p->poll_options_en[x]));
                    p->poll_count++;
                }
            }
        }
        p->group = (int)json_number(json_object_get(o, "grupo"), 0);
        if (p->group < 0 || p->group >= GRP_COUNT) p->group = 0;
        snprintf(p->date, sizeof(p->date), "%s", json_string(json_object_get(o, "data"), ""));
        snprintf(p->updated, sizeof(p->updated), "%s", json_string(json_object_get(o, "atualizado_em"), ""));
        p->feat = json_bool(json_object_get(o, "destaque"), 0);
        p->pixel = json_bool(json_object_get(o, "pixel"), 0);
        p->progress = (int)json_number(json_object_get(o, "progresso"), -1);
        const char *video = json_string(json_object_get(o, "video"), "");
        p->video = video[0] ? dup_latin1(video) : NULL;
        const char *vf = json_string(json_object_get(o, "video_arquivo"), "");
        p->video_file = vf[0] ? dup_latin1(vf) : NULL;
        p->trophies = json_bool(json_object_get(o, "trofeus"), 0);
        p->author = S(o, "autor"); /* ausente em posts antigos: string vazia */
    const char *kind = json_string(json_object_get(o, "novidade"), "");
    p->news_kind = !strcmp(kind, "Release") ? 1 : !strcmp(kind, "Update") ? 2 : 0;
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
            x->type = !strcmp(tp, "traducao") ? DL_TRANS : !strcmp(tp, "app_arquivos") ? DL_APP_FILES : (!strcmp(tp, "plugin") && json_bool(json_object_get(d, "instalar"), 0)) ? DL_TAI_PLUGIN : !strcmp(tp, "tai_plugin") ? DL_TAI_PLUGIN : !strcmp(tp, "plugin") ? DL_PLUGIN : !strcmp(tp, "arquivo") ? DL_FILE : !strcmp(tp, "dados") ? DL_DATA : DL_VPK;
            x->name = S(d, "nome");
            x->url = S(d, "url");
            x->version = S(d, "versao");
            x->file = S(d, "arquivo");
            x->instr = S(d, "instrucoes");
            x->plugin = S(d, "plugin");
            x->advisory_plugin = S(d, "aviso_plugin");
            x->password = S(d, "senha");
            const JsonValue *mirrors = json_object_get(d, "alternativas");
            for (int z = 0; z < json_array_count(mirrors) && x->nmirrors < MAX_MIRRORS; z++) {
                const char *u = json_string(json_array_get(mirrors, z), "");
                if (!strncmp(u, "https://", 8)) x->mirrors[x->nmirrors++] = dup_latin1(u);
            }
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
