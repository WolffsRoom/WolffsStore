#ifndef WS_CATALOG_H
#define WS_CATALOG_H

/* Catálogo gerado por tools/gen_data.py a partir dos posts do site.
 * Textos em Latin-1 (a fonte em bitmap cobre 32..255). */

enum { LANG_EN = 0, LANG_PT = 1 };
enum { CAT_PORTS, CAT_TOOLS, CAT_TRANS, CAT_REVIEWS, CAT_COUNT };
enum { GRP_DOING, GRP_DONE, GRP_NEXT, GRP_OTHERS, GRP_COUNT };
enum { BLK_SEC, BLK_PARA, BLK_ITEM };

#define MAX_POSTS 128
#define MAX_BLOCKS 96
#define MAX_PAIRS 24

typedef struct { int type; char *text; } Block;
typedef struct { char *a, *b, *c; int pct; } Pair; /* info: a|b; link: a|b(url); bar: a|pct|c */

typedef struct {
    char *title, *tag, *status, *desc, *feature, *bars_title, *about;
    Block blocks[MAX_BLOCKS]; int nblocks;
    Pair bars[MAX_PAIRS]; int nbars;
    Pair info[MAX_PAIRS]; int ninfo;
    Pair links[MAX_PAIRS]; int nlinks;
    Pair srcs[MAX_PAIRS]; int nsrcs;    /* caixa Source */
    Pair clock[MAX_PAIRS]; int nclock;  /* caixa Clock recomendado */
} PostText;

enum { DL_VPK, DL_TRANS, DL_PLUGIN, DL_FILE, DL_DATA, DL_APP_FILES }; /* DL_DATA: zip extraído em ux0:data/ */
#define MAX_DLS 8
#define MAX_MIRRORS 3
#define MAX_SHOTS 16

typedef struct {
    int type;
    char *name, *url, *version, *file, *instr, *plugin, *advisory_plugin;
    char *mirrors[MAX_MIRRORS]; int nmirrors;
    char *password;  /* optional password for encrypted ZIP translation */
    long long size;
    int needs_data;
    char tids[8][10];
    int ntids;
} Download;

typedef struct {
    char slug[48];
    int cat, group, feat, pixel, progress, cover, shots;
    char date[16];       /* publicação ISO YYYY-MM-DD */
    char updated[16];    /* última alteração do post; vazio se não houver */
    char *video;
    int trophies;     /* troféus nativos (NoTrpDrm) */
    char *author;     /* crédito opcional: autor do port externo */
    int news_kind;    /* 0=none, 1=Release, 2=Update (not an install status) */
    char *video_file; /* .mp4 que o app toca dentro dele (vazio = abre o YouTube) */
    PostText t[2];
    /* v2 (store.json): imagens no cache do cartão; vazio = usa as do VPK (data/posts) */
    char *cover_url, *shot_url[MAX_SHOTS];
    Download dls[MAX_DLS];
    int ndls;
} Post;

extern Post g_posts[MAX_POSTS];
extern int g_nposts;

int catalog_load(const char *path);
/* store.json (v2): preenche out[] e o app; devolve o número de posts ou -1 */
typedef struct {
    char version[24], url[512], notes[512];
    long long size;
} AppRelease;
int store_parse(const char *json_text, Post *out, int max, AppRelease *app);
/* caminho local de uma imagem do store.json ("store/pizza/capa-1a2b.png" -> ux0:data/.../cache/...) */
void store_cache_path(const char *rel, char *out, int n);
int catalog_featured(void);

#endif
