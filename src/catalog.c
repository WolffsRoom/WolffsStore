#include "catalog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Post g_posts[MAX_POSTS];
int g_nposts;

static char *dup(const char *s) {
    size_t n = strlen(s);
    char *d = malloc(n + 1);
    if (d) memcpy(d, s, n + 1);
    return d;
}

/* Divide "a|b|c" em até três partes (o texto original é copiado). */
static Pair split(const char *v) {
    Pair p = {0};
    char *s = dup(v), *x = strchr(s, '|');
    p.a = s;
    if (x) {
        *x = 0;
        p.b = x + 1;
        char *y = strchr(p.b, '|');
        if (y) {
            *y = 0;
            p.c = y + 1;
        }
    }
    if (!p.b) p.b = s + strlen(s);
    if (!p.c) p.c = p.b + strlen(p.b);
    return p;
}

static int cat_id(const char *v) {
    if (!strcmp(v, "ports")) return CAT_PORTS;
    if (!strcmp(v, "ferramentas")) return CAT_TOOLS;
    if (!strcmp(v, "traducoes")) return CAT_TRANS;
    if (!strcmp(v, "journal") || !strcmp(v, "reviews")) return CAT_REVIEWS;
    return CAT_REVIEWS;
}

int subcat_id(const char *v) {
    if (!strcmp(v, "apps")) return SUB_APP;
    if (!strcmp(v, "ferramentas")) return SUB_TOOL;
    if (!strcmp(v, "modificacoes")) return SUB_MOD;
    if (!strcmp(v, "plugins")) return SUB_PLUGIN;
    if (!strcmp(v, "news")) return SUB_NEWS;
    if (!strcmp(v, "enquetes")) return SUB_POLL;
    return SUB_REVIEW;
}

int catalog_load(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    static char line[8192];
    Post *p = NULL;
    g_nposts = 0;
    while (fgets(line, sizeof(line), f)) {
        size_t n = strlen(line);
        while (n && (line[n - 1] == '\n' || line[n - 1] == '\r')) line[--n] = 0;
        char *tab = strchr(line, '\t');
        if (!tab) continue;
        *tab = 0;
        const char *k = line, *v = tab + 1;
        if (!strcmp(k, "post")) {
            if (g_nposts >= MAX_POSTS) break;
            p = &g_posts[g_nposts++];
            memset(p, 0, sizeof(*p));
            p->progress = -1;
            snprintf(p->slug, sizeof(p->slug), "%s", v);
            continue;
        }
        if (!p) continue;
        if (!strcmp(k, "cat")) p->cat = cat_id(v);
        else if (!strcmp(k, "subcat")) p->subcat = subcat_id(v);
        else if (!strcmp(k, "poll_id")) snprintf(p->poll_id,sizeof(p->poll_id),"%s",v);
        else if (!strcmp(k, "poll_question")) snprintf(p->poll_question,sizeof(p->poll_question),"%s",v);
        else if (!strcmp(k, "poll_question.en")) snprintf(p->poll_question_en,sizeof(p->poll_question_en),"%s",v);
        else if (!strcmp(k, "poll_starts_at")) snprintf(p->poll_starts_at,sizeof(p->poll_starts_at),"%s",v);
        else if (!strcmp(k, "poll_ends_at")) snprintf(p->poll_ends_at,sizeof(p->poll_ends_at),"%s",v);
        else if (!strcmp(k, "poll_option") && p->poll_count < 8)
            snprintf(p->poll_options[p->poll_count++],112,"%s",v);
        else if (!strcmp(k, "poll_option.en")) {
            int idx=0;
            while(idx<8 && p->poll_options_en[idx][0])idx++;
            if(idx<8)snprintf(p->poll_options_en[idx],112,"%s",v);
        }
        else if (!strcmp(k, "group")) p->group = atoi(v);
        else if (!strcmp(k, "date")) snprintf(p->date, sizeof(p->date), "%s", v);
        else if (!strcmp(k, "updated")) snprintf(p->updated, sizeof(p->updated), "%s", v);
        else if (!strcmp(k, "feat")) p->feat = 1;
        else if (!strcmp(k, "pixel")) p->pixel = 1;
        else if (!strcmp(k, "trofeus")) p->trophies = 1;
        else if (!strcmp(k, "video_arquivo")) p->video_file = dup(v);
        else if (!strcmp(k, "progress")) p->progress = atoi(v);
        else if (!strcmp(k, "cover")) p->cover = 1;
        else if (!strcmp(k, "shots")) p->shots = atoi(v);
        else if (!strcmp(k, "video")) p->video = dup(v);
        else {
            /* chaves com idioma: campo.pt / campo.en */
            char *dot = strrchr(line, '.');
            if (!dot) continue;
            *dot = 0;
            int lg = !strcmp(dot + 1, "pt") ? LANG_PT : LANG_EN;
            PostText *t = &p->t[lg];
            if (!strcmp(k, "title")) t->title = dup(v);
            else if (!strcmp(k, "tag")) t->tag = dup(v);
            else if (!strcmp(k, "status")) t->status = dup(v);
            else if (!strcmp(k, "desc")) t->desc = dup(v);
            else if (!strcmp(k, "feature_text")) t->feature = dup(v);
            else if (!strcmp(k, "bars_title")) t->bars_title = dup(v);
            else if (!strcmp(k, "about")) t->about = dup(v);
            else if ((!strcmp(k, "sec") || !strcmp(k, "para") || !strcmp(k, "item")) && t->nblocks < MAX_BLOCKS) {
                Block *b = &t->blocks[t->nblocks++];
                b->type = k[0] == 's' ? BLK_SEC : k[0] == 'p' ? BLK_PARA : BLK_ITEM;
                b->text = dup(v);
            } else if (!strcmp(k, "bar") && t->nbars < MAX_PAIRS) {
                Pair q = split(v);
                q.pct = atoi(q.b);
                t->bars[t->nbars++] = q;
            } else if (!strcmp(k, "info") && t->ninfo < MAX_PAIRS) {
                t->info[t->ninfo++] = split(v);
            } else if (!strcmp(k, "link") && t->nlinks < MAX_PAIRS) {
                t->links[t->nlinks++] = split(v);
            }
        }
    }
    fclose(f);
    /* Campos vazios caem no outro idioma, como no site. */
    for (int i = 0; i < g_nposts; i++) {
        PostText *a = &g_posts[i].t[0], *b = &g_posts[i].t[1];
        for (int r = 0; r < 2; r++) {
            PostText *x = r ? b : a, *y = r ? a : b;
            if (!x->title) x->title = y->title ? y->title : g_posts[i].slug;
            if (!x->tag) x->tag = y->tag ? y->tag : "";
            if (!x->status) x->status = y->status ? y->status : "";
            if (!x->desc) x->desc = y->desc ? y->desc : "";
            if (!x->feature) x->feature = y->feature ? y->feature : x->desc;
            if (!x->about) x->about = y->about ? y->about : "";
        }
    }
    return g_nposts;
}

int catalog_featured(void) {
    for (int i = 0; i < g_nposts; i++)
        if (g_posts[i].feat) return i;
    return 0;
}
