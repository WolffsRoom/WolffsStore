/* Wolff's Store - o site Wolff's Room no PS Vita.
 * Mesmo visual, fontes, animações e sons do site (repo WolffsRoomSite). */
#include <psp2/appmgr.h>
#include <psp2/common_dialog.h>
#include <psp2/ctrl.h>
#include <psp2/ime_dialog.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/sysmodule.h>
#include <psp2/touch.h>
#include <vita2d.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "audio.h"
#include "catalog.h"
#include "fsutil.h"
#include "gfx.h"
#include "install.h"
#include "jobs.h"
#include "video.h"
#include "net.h"
#include "registry.h"
#include "sync.h"

#ifndef WS_VERSION
#define WS_VERSION "0.99.7"
#endif

/* ================================================================ textos */

enum {
    S_CAT0, S_CAT1, S_CAT2, S_CAT3,
    S_GRP0, S_GRP1, S_GRP2, S_GRP3,
    S_NOTHING, S_BACK, S_BACK_HOME, S_REPOS, S_SHOTS, S_VIDEO, S_WATCH, S_VIDEO_NOTE,
    S_INFO, S_LINKS, S_TOTAL, S_PROGRESS, S_LATEST, S_SEE_ALL, S_DETAILS, S_GITHUB,
    S_START, S_TAP, S_KICKER, S_SUBTITLE, S_COPY, S_H_OPEN, S_H_BACK, S_H_TOP, S_H_LANG, S_H_SOUND,
    S_H_SCROLL, S_H_CLOSE, S_H_NEXT,
    S_DOWNLOADS, S_DL_INSTALL, S_DL_UPDATE, S_DL_REINSTALL, S_DL_TRANS, S_DL_TRANS_RE, S_DL_GET, S_DATA_WARN, S_INSTR,
    S_REPATCH_OK, S_REPATCH_NO, S_TRANS_TO, S_PLUGIN_NOTE, S_INSTALLED_V, S_QUEUED, S_DOWNLOADING, S_EXTRACTING,
    S_INSTALLING, S_DONE, S_ERROR, S_CANCELED, S_CANCEL, S_CLEAR, S_NO_JOBS, S_SYNCING, S_SYNC_OK, S_SYNC_OFF,
    S_SYNC_FAIL, S_UPD_TITLE, S_UPD_SKIP, S_SAVED_TO, S_H_DLS, S_H_SYNC, S_T_INSTALLED, S_T_TRANS, S_T_SAVED,
    S_T_FAILED, S_UPD_RESTART, S_CHIP_UPDATE, S_CHIP_TRANS, S_SEE_DLS, S_BACK_TO,
    S_HOME, S_TIP_SYNC, S_TIP_DLS, S_TIP_LANG, S_TIP_SOUND_ON, S_TIP_SOUND_OFF, S_PROJECTS,
    S_TIP_SET, S_TIP_SEARCH, S_SETTINGS, S_SET_SFX, S_SET_MUSIC, S_SET_OST, S_SET_LANG, S_SET_BRIGHT, S_ON, S_OFF, S_LANG_NAME, S_AUTO_OST, S_SET_CAT_AUDIO, S_SET_CAT_VISUAL, S_SET_CAT_GENERAL,
    S_SET_HINT, S_SET_KEYS, S_H_MOVE, S_H_SELECT, S_SEARCH, S_SEARCH_FOR, S_RESULTS, S_NO_RESULTS, S_SEARCH_EMPTY,
    S_EDIT_SEARCH, S_ST_UPTODATE, S_ST_NEWVER, S_ST_INSTALLED, S_CHIP_INSTALLED, S_INSTALLED_LIST, S_OPEN_PROJECT,
    S_QUEUE, S_SOURCE, S_CLOCK, S_DL_DATA, S_DATA_TO, S_SYNC_SHORT, S_MUSIC, S_SFX, S_TIP_AUDIO, S_VID_LOAD,
    S_VID_FAIL, S_VID_HINT, S_BARK, S_WATCH_APP, S_VIDEO_NOTE_APP, S_LATEST3, S_TROPHIES, S_OPEN_APP, S_LBL_SYNC, S_LBL_DLS, S_LBL_AUDIO, S_LBL_SETTINGS, S_LBL_SEARCH, S_COUNT
};

static const char *UI_UTF8[2][S_COUNT] = {
    [LANG_EN] = {
        "Ports", "Tools", "Translations", "Reviews",
        "In progress", "Finished", "Upcoming", "Other teams",
        "Nothing here yet.", "Back", "Back to home", "All repositories on GitHub", "Screenshots", "Video",
        "Watch on YouTube", "The video opens in the browser.",
        "Details", "Downloads", "Total", "Progress", "Latest in", "See all", "See details", "View on GitHub",
        "Press start", "or tap the screen", "What could Wolff be up to...?",
        "Translations, Mods and much more.", "© 2026 Wolff's Room. All rights reserved.",
        "Open", "Back", "Top", "Language", "Sound", "Scroll", "Close", "Next",
        "Downloads", "Download and install", "Update", "Reinstall", "Install translation", "Reinstall translation",
        "Download", "This game needs the game's data files, check the instructions on GitHub.", "Instructions on GitHub",
        "rePatch plugin found.", "Needs the rePatch plugin: install it and restart the Vita.", "Goes to",
        "The file goes to ux0:Downloads. Follow the instructions on GitHub to install it.", "Installed: v",
        "Queued", "Downloading", "Extracting", "Installing", "Done", "Error", "Canceled", "Cancel", "Clear finished",
        "No downloads yet. Open a project and pick a download.", "Syncing...", "Catalog updated",
        "Offline: using the saved catalog", "Couldn't sync the catalog", "Updating Wolff's Store", "Skip for now",
        "Saved to", "Downloads", "Sync", "installed", "translation installed", "saved to ux0:Downloads", "failed",
        "The app will close and reopen on its own.", "Update", "Translation", "See downloads", "Back",
        "Home", "Sync", "Downloads", "Language: English", "Sound: on", "Sound: off", "projects",
        "Settings", "Search", "Settings", "Sound effects", "Background music", "Background OST", "Language", "Brightness", "On", "Off",
        "English", "Automatic", "Audio", "Visual", "General", "Left/Right: change", "Shortcuts: Triangle sync \xb7 Square downloads \xb7 SELECT search \xb7 START settings",
        "Move", "Select", "Search", "Search for", "results", "Nothing found. Try another word.",
        "Search ports, translations, tools and reviews.", "Edit search", "Up to date", "New version: v", "Installed",
        "Installed", "Installed", "Open project", "Queue",
        "Source", "Recommended clock", "Download data", "Extracted to ux0:data/", "Syncing...", "Music", "Effects", "Audio",
        "Loading video", "Couldn't play the video.", "X pause \xb7 Left/Right 10 s \xb7 O close", "WOOF!", "Watch here",
        "The video plays right here in the app.", "Check out the latest three updates...", "Native trophies", "Open", "Sync", "Downloads", "Audio", "Settings", "Search",
    },
    [LANG_PT] = {
        "Ports", "Ferramentas", "Traduções", "Reviews",
        "Em andamento", "Finalizado", "Próximos", "Outras equipes",
        "Nada por aqui ainda.", "Voltar", "Voltar ao início", "Todos os repositórios no GitHub",
        "Capturas de tela", "Vídeo", "Assistir no YouTube", "O vídeo abre no navegador.",
        "Detalhes", "Downloads", "Total", "Progresso", "Últimos em", "Ver todos", "Ver detalhes", "Ver no GitHub",
        "Pressione start", "ou toque na tela", "O que será que o Wolff está fazendo...?",
        "Traduções, Modificações e muito mais.", "© 2026 Wolff's Room. Todos os direitos reservados.",
        "Abrir", "Voltar", "Topo", "Idioma", "Som", "Rolar", "Fechar", "Trocar",
        "Downloads", "Baixar e instalar", "Atualizar", "Reinstalar", "Instalar tradução", "Reinstalar tradução",
        "Baixar", "Esse jogo precisa dos arquivos de dados do jogo, confira as instruções no GitHub.",
        "Instruções no GitHub", "Plugin rePatch encontrado.",
        "Precisa do plugin rePatch: instale-o e reinicie o Vita.", "Vai para",
        "O arquivo vai para ux0:Downloads. Siga as instruções no GitHub para instalar.", "Instalado: v",
        "Na fila", "Baixando", "Extraindo", "Instalando", "Concluído", "Erro", "Cancelado", "Cancelar",
        "Limpar concluídos", "Nenhum download por enquanto. Abra um projeto e escolha um download.",
        "Sincronizando...", "Catálogo atualizado", "Sem internet: usando o catálogo salvo",
        "Não deu para sincronizar o catálogo", "Atualizando o Wolff's Store", "Pular por agora", "Salvo em",
        "Downloads", "Sincronizar", "instalado", "tradução instalada", "salvo em ux0:Downloads", "falhou",
        "O app vai fechar e abrir de novo sozinho.", "Atualizar", "Tradução", "Ver downloads", "Voltar",
        "Início", "Sincronizar", "Downloads", "Idioma: português", "Som: ligado", "Som: desligado", "projetos",
        "Configurações", "Pesquisar", "Configurações", "Efeitos sonoros", "Música de fundo", "Background OST", "Idioma", "Brilho", "Ligado",
        "Desligado", "Português (BR)", "Automático", "Áudio", "Visual", "Geral", "Esquerda/direita: muda",
        "Atalhos: Triângulo sincroniza \xb7 Quadrado downloads \xb7 SELECT pesquisa \xb7 START configurações",
        "Mover", "Selecionar", "Pesquisa", "Pesquisar", "resultados", "Nada encontrado. Tente outra palavra.",
        "Procure ports, traduções, ferramentas e reviews.", "Editar pesquisa", "Atualizado", "Nova versão: v", "Instalado",
        "Instalado", "Instalados", "Abrir projeto", "Fila",
        "Source", "Clock recomendado", "Baixar dados", "Extraído em ux0:data/", "Sincron...", "Música", "Efeitos", "Áudio",
        "Carregando vídeo", "Não deu para tocar o vídeo.", "X pausa \xb7 Esq./Dir. 10 s \xb7 O fecha", "AU AU!", "Assistir aqui",
        "O vídeo toca aqui mesmo no app.", "Confira as últimas três novidades...", "Troféus nativos", "Abrir", "Sincronizar", "Downloads", "Áudio", "Configurações", "Pesquisar",
    },
};
static char UI[2][S_COUNT][128];

static int lang = LANG_EN;
#define TXT(k) (UI[lang][k])

/* ================================================================ estado */

enum { V_HOME, V_CAT, V_POST, V_DLS, V_SEARCH };
#define BADGE 52 /* botões do topo */
static int view = V_HOME, view_cat = 0, view_post = -1, from_view = V_HOME, peek = -1;
static int dls_from = V_HOME, dls_from_arg = 0;
static float hint_h = 36; /* barra de dicas (+ faixa de status quando aparece) */
static int brightness = 100;  /* % (escurece a imagem do app; 40..100) */
static int settings_open = 0, settings_row = 0, settings_page = 0, settings_root_row = 0;
static double settings_t0 = -10;
static char search_q[128] = "";
static int ime_open = 0;
static int sync_manual = 0, sync_reported = 1;
static AppRelease app_rel;
static int upd_job = -1, upd_skipped = 0; /* atualização do próprio app */
static int updater_cleanup_pending = 0;
static int legacy_updater_cleanup_pending = 0;
static double letter_hop[16] = {-100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100};
static double bark_t0 = -100, sync_done_t = -100;
static int snd_menu = 0, snd_row = 0, vid_open = 0, vid_music = 0;
static double now_s, view_t0, peek_t0, intro_t0 = -1, wipe_t0 = -100, wave_t0 = -100, pressed_t0 = -100;
static int started = 0, pressed_item = -1;
static int first_sync = 0;
static double title_t0 = 0;
static double home_t0 = -100; /* animação de volta ao início */
static int feat = 0;

static vita2d_texture *cover_tex[MAX_POSTS];
static vita2d_texture *mos_tex[8];
static const int MOS_W[8] = {12, 20, 32, 48, 72, 112, 176, 288};
static vita2d_texture *shot_tex[16];
static int shot_count = 0, viewer = -1;

/* Linha do tempo da intro, em segundos depois do toque/START */
#define T_TV    .30   /* TV liga com a capa do projeto atual, grande no centro */
#define T_MOS   .58   /* depixelização: 8 camadas, uma a cada .13 s */
#define T_HALO  1.60  /* halo de pixels em volta */
#define T_MOVE  2.20  /* a capa encaixa no lugar dela na página (.5 s) */
#define T_HEAD  2.50  /* topo com a pata e o logo */
#define T_INFO  2.80  /* textos do destaque */
#define T_NAV   3.20  /* categorias, uma a cada .1 s */
#define T_UI    3.70  /* barra de dicas e botões de idioma/som */
#define T_DONE  4.30

static double clock_s(void) { return sceKernelGetProcessTimeWide() / 1000000.0; }

/* ================================================================ lista de desenho */

enum { OP_RECT, OP_FRAME, OP_TITLE, OP_BODY, OP_TEX, OP_HALO, OP_BAR, OP_STAGE, OP_LOGO, OP_PLAY, OP_TRI,
       OP_BADGE, OP_DLJOB, OP_JOBROW, OP_WARN, OP_HLABEL, OP_ICON, OP_TROPHY };
enum { A_NONE, A_POST, A_PEEK, A_ALL, A_HOME, A_BACK, A_URL, A_SHOT, A_TOP, A_START,
       A_SYNC, A_DLS, A_LANG, A_SOUND, A_DL, A_CANCEL, A_CLEAR, A_SETTINGS, A_SEARCH, A_SEARCHBOX, A_REGUPD, A_VIDEO, A_LAUNCH };

typedef struct {
    int type, item, scale, bold;
    float x, y, w, h;
    unsigned c, cf;        /* cor normal / com foco */
    unsigned shadow;       /* sombra do título (0 = nenhuma) */
    float a, af;           /* halo: opacidade normal / com foco */
    const char *s;
    vita2d_texture *tex;
    double pop;            /* início da animação "pop" (<0 = sem) */
} Op;

typedef struct {
    float x, y, w, h;
    int act, arg;
    const char *url;
    double pop;
} Item;

#define MAX_OPS 4096
#define MAX_ITEMS 256
static Op ops[MAX_OPS];
static int nops;
static Item items[MAX_ITEMS];
static int nitems, focus = -1;
static float page_h = SCR_H, cam = 0, cam_target = 0;

static char arena[512 * 1024];
static int arena_used;

static const char *keep(const char *s) {
    int n = (int)strlen(s) + 1;
    if (arena_used + n > (int)sizeof(arena)) return "";
    char *d = arena + arena_used;
    memcpy(d, s, n);
    arena_used += n;
    return d;
}

static const char *keep_upper(const char *s) {
    char *d = (char *)keep(s);
    upper_latin1(d);
    return d;
}

static Op *op(int type, float x, float y, float w, float h) {
    if (nops >= MAX_OPS) {
        static Op dummy;
        return &dummy;
    }
    Op *o = &ops[nops++];
    memset(o, 0, sizeof(*o));
    o->type = type;
    o->x = x;
    o->y = y;
    o->w = w;
    o->h = h;
    o->item = -1;
    o->pop = -1;
    o->c = o->cf = C_WHITE;
    return o;
}

static int item(float x, float y, float w, float h, int act, int arg, const char *url) {
    if (nitems >= MAX_ITEMS) return -1;
    Item *it = &items[nitems];
    it->x = x;
    it->y = y;
    it->w = w;
    it->h = h;
    it->act = act;
    it->arg = arg;
    it->url = url;
    it->pop = -1;
    return nitems++;
}


/* ---------------------------------------------------------------- peças */

static Op *rect(float x, float y, float w, float h, unsigned c) {
    Op *o = op(OP_RECT, x, y, w, h);
    o->c = o->cf = c;
    return o;
}

static Op *frame(float x, float y, float w, float h, unsigned c, unsigned cf, int it) {
    Op *o = op(OP_FRAME, x, y, w, h);
    o->c = c;
    o->cf = cf;
    o->item = it;
    return o;
}

static Op *title(const char *s, float x, float y, int scale, unsigned c, unsigned shadow) {
    Op *o = op(OP_TITLE, x, y, title_width(s, scale), 8 * scale);
    o->s = s;
    o->scale = scale;
    o->c = o->cf = c;
    o->shadow = shadow;
    return o;
}

static Op *body(const char *s, float x, float y, unsigned c) {
    Op *o = op(OP_BODY, x, y, body_width(s), BODY_LINE);
    o->s = s;
    o->c = o->cf = c;
    return o;
}

static Op *halo(float x, float y, float w, float h, int big, float a, float af, int it) {
    /* .wr-sh::before inset -28 / .wr-sh-s::before inset -16 */
    float in = big ? 28 : 16;
    Op *o = op(OP_HALO, x - in, y - in, w + 2 * in, h + 2 * in);
    o->scale = big;
    o->a = a;
    o->af = af;
    o->item = it;
    return o;
}

/* Texto corrido com quebra de linha; align: 0 esquerda, 1 centro, 2 direita. Devolve a altura. */
static float para(const char *s, float x, float y, float w, unsigned c, int align, int maxlines, int bold) {
    char *lines[64];
    static char buf[8192];
    if (maxlines > 64) maxlines = 64;
    int n = wrap_body(s, (int)w, lines, maxlines, buf, sizeof(buf));
    for (int i = 0; i < n; i++) {
        int lw = body_width(lines[i]);
        float lx = align == 1 ? x + (w - lw) / 2 : align == 2 ? x + w - lw : x;
        Op *o = body(keep(lines[i]), (int)lx, y + i * BODY_LINE, c);
        o->bold = bold;
    }
    return n * BODY_LINE;
}

static int para_lines(const char *s, float w, int maxlines) {
    char *lines[64];
    static char buf[8192];
    return wrap_body(s, (int)w, lines, maxlines > 64 ? 64 : maxlines, buf, sizeof(buf));
}

/* Título em Press Start 2P com quebra; line = altura da linha. */
static float heading(const char *s, float x, float y, float w, int scale, unsigned c, unsigned shadow, int align, int upper, float line) {
    char *lines[16];
    static char buf[2048];
    const char *src = upper ? keep_upper(s) : s;
    int n = wrap_title(src, scale, (int)w, lines, 16, buf, sizeof(buf));
    for (int i = 0; i < n; i++) {
        int lw = title_width(lines[i], scale);
        float lx = align == 1 ? x + (w - lw) / 2 : align == 2 ? x + w - lw : x;
        title(keep(lines[i]), (int)lx, y + i * line, scale, c, shadow);
    }
    return n * line;
}

static int heading_lines(const char *s, int scale, float w) {
    char *lines[16];
    static char buf[2048];
    char up[1024];
    snprintf(up, sizeof(up), "%s", s);
    upper_latin1(up);
    return wrap_title(up, scale, (int)w, lines, 16, buf, sizeof(buf));
}

/* Botão do site: borda 4px, Press Start 16px em maiúsculas.
 * filled: fundo branco e texto escuro (botão principal, com sombra 4px). */
static int button(const char *label, float x, float y, float w, int filled, int act, int arg, const char *url, double pop) {
    const char *s = keep_upper(label);
    float tw = title_width(s, 2);
    if (w <= 0) w = tw + 48;
    float h = 48;
    int lines = 1;
    if (tw > w - 40) {
        lines = heading_lines(label, 2, w - 40);
        h = 24 + lines * 24;
    }
    int it = item(x, y, w, h, act, arg, url);
    if (it >= 0) items[it].pop = pop;
    if (filled) {
        Op *o = rect(x + 4, y + 4, w, h, C_SH1);
        o->item = it;
        o->pop = pop;
    }
    Op *bg = rect(x, y, w, h, filled ? C_WHITE : C_DARK);
    bg->item = it;
    bg->pop = pop;
    Op *fr = frame(x, y, w, h, filled ? C_WHITE : C_BTNBRD, C_WHITE, it);
    fr->pop = pop;
    int first = nops;
    heading(s, x + 20, y + 16, w - 40, 2, filled ? C_DARK : C_WHITE, 0, 1, 0, 24);
    for (int i = first; i < nops; i++) {
        ops[i].item = it;
        ops[i].pop = pop;
    }
    Op *hl = halo(x, y, w, h, 0, 0, 1, it);
    hl->pop = pop;
    return it;
}

/* ---------------------------------------------------------------- estado dos downloads */

typedef struct {
    int installed, update;
    char inst_ver[24];
    char tid[16]; /* VPK instalado: TitleID para o botão Abrir */
} DlStatus;

/* Estado de um download: o registro do que o app instalou (installed.txt) + o que existe no cartão.
 * Instalado por fora do app (VPK com o mesmo TitleID) também conta. */
static void dl_status(const Post *p, int idx, DlStatus *s) {
    const Download *d = &p->dls[idx];
    RegEntry *r = reg_find(p->slug, idx);
    memset(s, 0, sizeof(*s));
    if (d->type == DL_VPK) {
        char tid[16] = "";
        if (r && !strncmp(r->path, "ux0:app/", 8) && r->path[8]) snprintf(tid, sizeof(tid), "%s", r->path + 8);
        for (int i = 0; i <= d->ntids && !s->installed; i++) {
            const char *t = i == 0 ? tid : d->tids[i - 1];
            if (!t[0]) continue;
            char sfo[96], av[24] = "";
            snprintf(sfo, sizeof(sfo), "ux0:app/%s/sce_sys/param.sfo", t);
            if (sfo_get(sfo, "APP_VER", av, sizeof(av)) == 0) {
                s->installed = 1;
                snprintf(s->tid, sizeof(s->tid), "%s", t);
                /* O SFO é a fonte real, inclusive numa instalação limpa ou
                 * quando o VPK foi atualizado fora da Wolff's Store. */
                snprintf(s->inst_ver, sizeof(s->inst_ver), "%s", av);
            }
        }
    } else if (d->type == DL_TRANS) {
        for (int i = 0; i < d->ntids && !s->installed; i++) {
            char path[64];
            snprintf(path, sizeof(path), "ux0:rePatch/%s", d->tids[i]);
            s->installed = fs_is_dir(path);
        }
        if (s->installed && r) snprintf(s->inst_ver, sizeof(s->inst_ver), "%s", r->version);
    } else if (r && fs_exists(r->path)) {
        s->installed = 1;
        snprintf(s->inst_ver, sizeof(s->inst_ver), "%s", r->version);
    }
    if (s->installed && d->version[0] && s->inst_ver[0]) s->update = version_cmp(s->inst_ver, d->version) < 0;
}

/* plugin rePatch listado no config do taiHEN */
static int repatch_plugin(void) {
    return fs_grep_ci("ur0:tai/config.txt", "repatch") || fs_grep_ci("ux0:tai/config.txt", "repatch");
}

static void fmt_size(long long b, char *out, int n) {
    if (b <= 0) out[0] = 0;
    else if (b >= 1024LL * 1024 * 1024) snprintf(out, n, "%.1f GB", b / (1024.0 * 1024 * 1024));
    else if (b >= 1024 * 1024) snprintf(out, n, "%.1f MB", b / (1024.0 * 1024));
    else snprintf(out, n, "%lld KB", (b + 1023) / 1024);
}

/* ---------------------------------------------------------------- card */

#define CARD_W 288
#define COVER_W 280
#define COVER_H 158

/* Release/update is a yellow text suffix of the featured app title.
 * Never overlay it on navigation buttons or cards. */
static const char *feature_suffix(int kind) {
    return kind == 1 ? " - Release" : kind == 2 ? " - Update" : "";
}

/* Compute the same wrapped title height used by the renderer, reserving an
 * extra row only when the suffix cannot fit after the last title word. */
static int feature_title_rows(const char *name, int scale, float width, int kind) {
    char *lines[16], buf[2048];
    int n = wrap_title(name, scale, (int)width, lines, 16, buf, sizeof(buf));
    const char *suffix = feature_suffix(kind);
    if (n > 0 && suffix[0] &&
        title_width(lines[n - 1], scale) + 8 + title_width(suffix, 2) > width)
        n++;
    return n;
}

static float feature_title(const char *name, float x, float y, float width,
                           int scale, int kind, double pop) {
    char *lines[16], buf[2048];
    int n = wrap_title(name, scale, (int)width, lines, 16, buf, sizeof(buf));
    float line_h = scale * 8 + 12;
    for (int i = 0; i < n; i++) {
        Op *o = title(keep(lines[i]), x, y + i * line_h,
                      scale, C_WHITE, C_SHADOW);
        o->pop = pop;
    }
    const char *suffix = feature_suffix(kind);
    if (n > 0 && suffix[0]) {
        float sx = x + title_width(lines[n - 1], scale) + 8;
        float sy = y + (n - 1) * line_h + (scale > 2 ? 5 : 0);
        if (sx + title_width(suffix, 2) > x + width) {
            sx = x;
            sy = y + n * line_h;
            n++;
        }
        Op *o = title(keep(suffix), sx, sy, 2, C_GOLD, 0);
        o->pop = pop + .08; /* a subtle second pixel-art entrance */
    }
    return n * line_h;
}

static float card_height(int pi) {
    PostText *t = &g_posts[pi].t[lang];
    float h = 4 + COVER_H + 4 + 16;
    char tag[256];
    snprintf(tag, sizeof(tag), "%s", t->tag);
    upper_latin1(tag);
    h += para_lines(tag, CARD_W - 40, 2) * BODY_LINE + 10; /* tag */
    if (g_posts[pi].author && g_posts[pi].author[0]) {
        char by[128];
        snprintf(by, sizeof(by), "by: %s", g_posts[pi].author);
        h += para_lines(by, CARD_W - 56, 2) * BODY_LINE + 12 + 10;
    }
    h += heading_lines(t->title, 2, CARD_W - 40) * 24 + 10; /* título */
    h += para_lines(t->desc, CARD_W - 40, 6) * BODY_LINE + 10;
    h += BODY_LINE + 16 + 4;                               /* status */
    return h;
}

static void card(int pi, float x, float y, float h, double pop) {
    Post *p = &g_posts[pi];
    PostText *t = &p->t[lang];
    int it = item(x, y, CARD_W, h, A_POST, pi, NULL);
    if (it >= 0) items[it].pop = pop;
    int first = nops;
    rect(x, y, CARD_W, h, C_PANEL);
    if (cover_tex[pi]) {
        Op *o = op(OP_TEX, x + 4, y + 4, COVER_W, COVER_H);
        o->tex = cover_tex[pi];
        o->c = o->cf = C_WHITE;
    } else {
        rect(x + 4, y + 4, COVER_W, COVER_H, RGBA8(0x16, 0x16, 0x16, 0xFF));
        para(t->title, x + 16, y + 4 + COVER_H / 2 - BODY_LINE / 2, COVER_W - 24, C_MUTED, 1, 2, 0);
    }
    if (p->ndls) {
        /* etiqueta no canto da capa: atualização disponível ou o tipo do download */
        const char *chip = p->dls[0].type == DL_TRANS ? TXT(S_CHIP_TRANS) : p->dls[0].type == DL_PLUGIN ? "Plugin" : "VPK";
        for (int i = 0; i < p->ndls; i++) {
            DlStatus s;
            dl_status(p, i, &s);
            if (s.update) chip = TXT(S_CHIP_UPDATE);
            else if (s.installed && chip != TXT(S_CHIP_UPDATE)) chip = TXT(S_CHIP_INSTALLED);
        }
        const char *c = keep_upper(chip);
        float cw = body_width(c) + 20;
        rect(x + 12, y + 12, cw, 28, RGBA8(0x06, 0x06, 0x06, 0xE6));
        frame(x + 12, y + 12, cw, 28, C_WHITE, C_WHITE, -1)->bold = 2; /* borda fina */
        Op *t = body(c, x + 22, y + 13, C_WHITE);
        t->bold = 1;
    }
    if (p->trophies) { /* só o troféu em pixel art no canto da capa (troféus nativos, NoTrpDrm) */
        Op *o = op(OP_TROPHY, x + 4 + COVER_W - 12 - 30, y + 12, 30, 30);
        o->scale = 3;
    }
    rect(x + 4, y + 4 + COVER_H, COVER_W, 4, C_LINE);
    float cy = y + 4 + COVER_H + 4 + 16;
    char tag[256];
    snprintf(tag, sizeof(tag), "%s", t->tag);
    upper_latin1(tag);
    cy += para(tag, x + 20, cy, CARD_W - 40, C_MUTED, 1, 2, 0) + 10;
    if (p->author && p->author[0]) {
        char by[128];
        snprintf(by, sizeof(by), "by: %s", p->author);
        float bh = para_lines(by, CARD_W - 56, 2) * BODY_LINE + 12;
        rect(x + 20, cy, CARD_W - 40, bh, RGBA8(0x19, 0x19, 0x19, 0xFF));
        frame(x + 20, cy, CARD_W - 40, bh, C_BORDER, C_BORDER, -1);
        para(by, x + 28, cy + 5, CARD_W - 56, C_WHITE, 1, 2, 0);
        cy += bh + 10;
    }
    cy += heading(t->title, x + 20, cy, CARD_W - 40, 2, C_WHITE, 0, 1, 0, 24) + 10;
    cy += para(t->desc, x + 20, cy, CARD_W - 40, C_SOFT, 1, 6, 0) + 10;
    para(t->status, x + 20, y + h - 4 - 16 - BODY_LINE, CARD_W - 40, C_WHITE, 1, 1, 1);
    frame(x, y, CARD_W, h, C_BORDER, C_WHITE, it);
    halo(x, y, CARD_W, h, 0, 0, 1, it);
    for (int i = first; i < nops; i++) {
        ops[i].item = it;
        ops[i].pop = pop;
    }
}

/* Grade de 3 colunas; devolve a altura usada. */
static float grid(const int *ids, int n, float y, double pop0, double step) {
    float yy = y;
    for (int r = 0; r < n; r += 3) {
        float h = 0;
        for (int c = 0; c < 3 && r + c < n; c++) {
            float ch = card_height(ids[r + c]);
            if (ch > h) h = ch;
        }
        for (int c = 0; c < 3 && r + c < n; c++)
            card(ids[r + c], 24 + c * (CARD_W + 24), yy, h, pop0 < 0 ? -1 : pop0 + (r + c) * step);
        yy += h + 36;
    }
    return yy - y - (n ? 36 : 0);
}

/* ---------------------------------------------------------------- topo e rodapé */

static const char *LOGO = "WOLFF'S STORE";
#define LOGO_SC 3                 /* letras do logo: 24 px (abre espaço para os nomes dos botões do topo) */
#define LOGO_X 112                /* x da primeira letra */
#define LOGO_STEP (8 * LOGO_SC)

/* colunas dos botões do topo (ícone com o nome embaixo), calculadas em header() */
static float hdr_x[5], hdr_w[5];
static const int HDR_ACTS[5] = {A_SYNC, A_DLS, A_SOUND, A_SETTINGS, A_SEARCH};
#define HDR_COL_W 72
#define HDR_GAP 10

static void hdr_label(int act, char *t, int n) {
    if (act == A_SYNC) {
        int running = sync_state() == SYNC_RUNNING;
        snprintf(t, n, "%s", TXT(running || now_s - sync_done_t < 1.2 ? S_SYNC_SHORT : S_LBL_SYNC));
    } else if (act == A_DLS) {
        int k = jobs_active();
        if (k) snprintf(t, n, "%s (%d)", TXT(S_LBL_DLS), k);
        else snprintf(t, n, "%s", TXT(S_LBL_DLS));
    } else if (act == A_SOUND) snprintf(t, n, "%s", TXT(S_LBL_AUDIO));
    else if (act == A_SETTINGS) snprintf(t, n, "%s", TXT(S_LBL_SETTINGS));
    else snprintf(t, n, "%s", TXT(S_LBL_SEARCH));
}

static double intro_pop(double k) { return started ? intro_t0 + k : 1e9; }
/* Na volta ao início os elementos entram de novo, mais rápido que na intro */
static double home_pop(double k_intro, double k_home) {
    if (started && home_t0 >= intro_t0 + T_DONE) return home_t0 + k_home;
    return intro_pop(k_intro);
}

static float header(void) {
    Op *o = op(OP_LOGO, 24, 16, 0, 68);
    o->pop = intro_pop(T_HEAD);
    o = rect(24, 108, 912, 4, C_LINE);
    o->pop = intro_pop(T_HEAD + .08);
    /* botões do topo: sincronizar, downloads, áudio (abre Música/Efeitos), configurações, pesquisa.
     * Cada um é uma coluna com o ícone e o nome centralizado embaixo; as colunas vêm da direita. */
    float right = 936;
    for (int i = 4; i >= 0; i--) {
        hdr_w[i] = HDR_COL_W;
        hdr_x[i] = right - hdr_w[i];
        right = hdr_x[i] - HDR_GAP;
    }
    for (int i = 0; i < 5; i++) {
        float bx = hdr_x[i] + (hdr_w[i] - BADGE) / 2, by = 18;
        int it = item(hdr_x[i], by, hdr_w[i], BADGE + 26, HDR_ACTS[i], 0, NULL);
        if (it >= 0) items[it].pop = intro_pop(T_UI);
        o = op(OP_BADGE, bx, by, BADGE, BADGE);
        o->scale = HDR_ACTS[i];
        o->item = it;
        o->pop = intro_pop(T_UI + i * .05);
        o = op(OP_HLABEL, hdr_x[i] + hdr_w[i] / 2, by + BADGE - 2, 0, 0);
        o->scale = HDR_ACTS[i];
        o->item = it;
        o->pop = intro_pop(T_UI + i * .05);
    }
    return 112;
}

static float footer(float y) {
    y += 56;
    rect(24, y, 912, 4, C_LINE);
    y += 40;
    const char *name = "WOLFF'S ROOM";
    float w = title_width(name, 4);
    int it = item(SCR_W / 2 - w / 2 - 8, y - 8, w + 24, 48, A_TOP, 0, NULL);
    Op *o = title(name, SCR_W / 2 - w / 2, y, 4, C_WHITE, 0);
    o->item = it;
    o->bold = 2; /* sombra dupla do logo: 4px #3a3a3a e 8px #1a1a1a */
    halo(SCR_W / 2 - w / 2 - 8, y - 8, w + 24, 48, 0, 0, 1, it);
    y += 56;
    y += para(TXT(S_SUBTITLE), 24, y, 912, C_MUTED, 1, 2, 0) + 8;
    y += para(TXT(S_COPY), 24, y, 912, RGBA8(0x7A, 0x7A, 0x7A, 0xFF), 1, 2, 0);
    y += 8 + para("Wolff's Store v" WS_VERSION, 24, y + 8, 912, RGBA8(0x5A, 0x5A, 0x5A, 0xFF), 1, 1, 0);
    return y + 64; /* espaço para a barra de dicas */
}

/* ================================================================ telas */

static void layout_begin(void) {
    nops = 0;
    nitems = 0;
    arena_used = 0;
}

/* Anima (pop) tudo o que foi criado desde 'first' que ainda não tem animação própria */
static void pop_range(int first, double at) {
    for (int i = first; i < nops; i++)
        if (ops[i].pop < 0) ops[i].pop = at;
}

static int find_item(int act, int arg) {
    for (int i = 0; i < nitems; i++)
        if (items[i].act == act && items[i].arg == arg) return i;
    return -1;
}

static double nav_pop(int i) {
    /* .wr-cat aparece em 4,7 / 4,8 / 4,9 / 5,0 s depois do start */
    return home_pop(T_NAV + i * 0.1, .42 + i * .06);
}

static const char *vita_github_url(const char *url, char *out, size_t cap);

static void build_home(void) {
    layout_begin();
    float y = header() + 32;
    Post *p = &g_posts[feat];
    PostText *t = &p->t[lang];

    /* palco: capa do destaque com TV, mosaico e texto digitado */
    float sx = 40, sw = 448, sh = 252, sy = y;
    op(OP_STAGE, sx, sy, sw, sh);
    int st = item(sx, sy, sw, sh, started ? A_POST : A_START, feat, NULL);
    (void)st;

    /* informações à direita */
    float ix = 528, iw = 408;
    double ip = home_pop(T_INFO, .3);
    char line[512];
    snprintf(line, sizeof(line), "%s \xb7 %s", t->tag, t->status);
    upper_latin1(line);
    /* a coluna precisa caber na altura do palco (o resto fica longe das categorias) */
    int tsc = feature_title_rows(t->title, 3, iw, p->news_kind) <= 2 ? 3 : 2, fl = 5;
    float th, total;
    for (;;) {
        th = feature_title_rows(t->title, tsc, iw, p->news_kind) * (tsc * 8 + 12);
        int n = para_lines(t->feature, iw, fl);
        total = BODY_LINE + 10 + (p->author && p->author[0] ? BODY_LINE + 8 : 0)
              + th + 10 + n * BODY_LINE + 16 + 48;
        if (total <= sh || (tsc == 2 && fl <= 2)) {
            fl = n;
            break;
        }
        if (tsc == 3) tsc = 2;
        else fl--;
    }
    float iy = sy + (sh - total) / 2;
    if (iy < sy) iy = sy;
    int first = nops;
    iy += para(line, ix, iy, iw, C_MUTED, 0, 1, 0) + 10;
    for (int i = first; i < nops; i++) ops[i].pop = ip;
    if (p->author && p->author[0]) {
        char by[128];
        snprintf(by, sizeof(by), "by: %s", p->author);
        first = nops;
        iy += para(by, ix, iy, iw, C_GOLD, 0, 2, 0) + 8;
        for (int i = first; i < nops; i++) ops[i].pop = ip + .04;
    }
    iy += feature_title(t->title, ix, iy, iw, tsc,
                        p->news_kind, ip + .08) + 10;
    first = nops;
    iy += para(t->feature, ix, iy, iw, C_SOFT, 0, fl, 0) + 16;
    for (int i = first; i < nops; i++) ops[i].pop = ip + 0.16;
    int b1 = button(TXT(S_DETAILS), ix, iy, 0, 1, A_POST, feat, NULL, ip + 0.24);
    /* A source repository is not the same as a release VPK download. */
    const char *gh = NULL;
    for (int i = 0; i < t->nsrcs && !gh; i++) {
        const char *url = t->srcs[i].b;
        if (url && strstr(url, "github.com/") && !strstr(url, "/releases/download/"))
            gh = url;
    }
    for (int i = 0; i < t->nlinks && !gh; i++) {
        const char *url = t->links[i].b;
        if (url && strstr(url, "github.com/") && !strstr(url, "/releases/"))
            gh = url;
    }
    if (gh && b1 >= 0) {
        char mapped[520];
        const char *url = vita_github_url(gh, mapped, sizeof(mapped));
        button("GitHub", items[b1].x + items[b1].w + 16, iy, 0, 0,
               A_URL, 0, keep(url), ip + 0.32);
    }
    /* as três novidades mais recentes (o catálogo já vem do mais novo para o mais antigo) */
    y = sy + sh + 44;
    {
        int ids[3], n = 0;
        for (int i = 0; i < g_nposts && n < 3; i++) ids[n++] = i;
        if (n) {
            int f0 = nops;
            y += heading(TXT(S_LATEST3), 24, y, 912, 2, C_SOFT, 0, 1, 1, 24) + 24;
            for (int i = f0; i < nops; i++) ops[i].pop = nav_pop(0);
            y += grid(ids, n, y, nav_pop(0) + .05, .08) + 44;
        }
    }

    /* categorias */
    float bw = (912 - 3 * 32) / 4.f;
    for (int c = 0; c < CAT_COUNT; c++) {
        float bx = 24 + c * (bw + 32);
        int it = item(bx, y, bw, 56, A_PEEK, c, NULL);
        if (it >= 0) items[it].pop = nav_pop(c);
        int sel = peek == c;
        Op *o = rect(bx, y, bw, 56, sel ? C_WHITE : C_BTN);
        o->item = it;
        o->pop = nav_pop(c);
        o = frame(bx, y, bw, 56, sel ? C_WHITE : C_BTNBRD, C_WHITE, it);
        o->pop = nav_pop(c);
        int f0 = nops;
        heading(TXT(S_CAT0 + c), bx + 4, y + 20, bw - 8, 2, sel ? C_DARK : C_WHITE, 0, 1, 1, 24);
        for (int i = f0; i < nops; i++) {
            ops[i].item = it;
            ops[i].pop = nav_pop(c);
        }
        o = halo(bx, y, bw, 56, 0, .75f, 1, it); /* .wr-sh-s: 75% parado, 100% com foco */
        o->pop = nav_pop(c);
    }
    y += 56;

    /* "Últimos em ..." */
    if (peek >= 0) {
        y += 48;
        char h[128];
        snprintf(h, sizeof(h), "%s %s", TXT(S_LATEST), TXT(S_CAT0 + peek));
        y += heading(h, 24, y, 912, 2, C_SOFT, 0, 1, 1, 24) + 24;
        int ids[3], n = 0, total = 0;
        for (int i = 0; i < g_nposts; i++)
            if (g_posts[i].cat == peek) {
                if (n < 3) ids[n++] = i;
                total++;
            }
        if (n) {
            y += grid(ids, n, y, peek_t0, 0.1) + 24;
        } else {
            y += para(TXT(S_NOTHING), 24, y, 912, C_MUTED, 1, 1, 0) + 24;
        }
        char all[64];
        snprintf(all, sizeof(all), "%s (%d)", TXT(S_SEE_ALL), total);
        float w = title_width(all, 2) + 48;
        button(all, SCR_W / 2 - w / 2, y, w, 1, A_ALL, peek, NULL, peek_t0 + .3);
        y += 48;
    }
    page_h = footer(y);
}

/* Barra de navegação das páginas internas: botão voltar à esquerda e o caminho (migalhas) ao lado. */
static float page_bar(float y, int act, const char *c1, const char *c2) {
    const char *lbl = keep_upper(TXT(S_BACK));
    float tw = title_width(lbl, 2), w = 20 + 16 + 12 + tw + 20, h = 48;
    int it = item(24, y, w, h, act, 0, NULL);
    int first = nops;
    rect(24, y, w, h, C_DARK);
    frame(24, y, w, h, C_BTNBRD, C_WHITE, it);
    Op *t = op(OP_TRI, 24 + 20, y + 16, 16, 16);
    t->c = C_WHITE;
    t->cf = C_WHITE;
    title(lbl, 24 + 20 + 16 + 12, y + 16, 2, C_WHITE, 0);
    for (int i = first; i < nops; i++) ops[i].item = it;
    halo(24, y, w, h, 0, 0, 1, it);
    /* migalhas: Início » categoria » projeto */
    char crumb[256];
    snprintf(crumb, sizeof(crumb), "%s \xbb %s%s%s", TXT(S_HOME), c1, c2 ? " \xbb " : "", c2 ? c2 : "");
    para(crumb, 24 + w + 24, y + 11, 936 - (24 + w + 24), C_MUTED, 0, 1, 0);
    return y + h;
}

static void build_cat(int c) {
    layout_begin();
    float y = page_bar(header() + 24, A_HOME, TXT(S_CAT0 + c), NULL) + 32;
    char h[64];
    snprintf(h, sizeof(h), "%s", TXT(S_CAT0 + c));
    upper_latin1(h);
    title(keep(h), 24, y, 4, C_WHITE, C_SHADOW);
    int total = 0;
    for (int i = 0; i < g_nposts; i++) total += g_posts[i].cat == c;
    char cnt[48];
    snprintf(cnt, sizeof(cnt), "%d %s", total, TXT(S_PROJECTS));
    para(cnt, 24, y + 4, 912, C_MUTED, 2, 1, 0);
    y += 56;
    rect(24, y, 912, 4, C_LINE);
    y += 4 + 44;
    int has_others = 0;
    for (int i = 0; i < g_nposts; i++)
        if (g_posts[i].cat == c && g_posts[i].group == GRP_OTHERS) has_others = 1;
    for (int g = 0; g < (has_others ? 4 : 3); g++) {
        int ids[MAX_POSTS], n = 0;
        for (int i = 0; i < g_nposts; i++)
            if (g_posts[i].cat == c && g_posts[i].group == g) ids[n++] = i;
        char gh[96];
        snprintf(gh, sizeof(gh), "%s (%d)", TXT(S_GRP0 + g), n);
        y += heading(gh, 24, y, 912, 2, C_SOFT, 0, 1, 1, 24) + 28;
        if (n) y += grid(ids, n, y, view_t0 + .12 + g * .1, .06);
        else y += para(TXT(S_NOTHING), 24, y, 912, C_MUTED, 1, 1, 0);
        y += 44;
    }
    if (c == CAT_PORTS || c == CAT_TOOLS) {
        float w = title_width(keep_upper(TXT(S_REPOS)), 2) + 48;
        if (w > 912) w = 912;
        button(TXT(S_REPOS), SCR_W / 2 - w / 2, y, w, 0, A_URL, 0, "https://wolffsroom.github.io/WolffsStore/", -1);
        y += 48;
    }
    page_h = footer(y);
}

/* Caixa da coluna lateral (.wr-sh-s com borda #5a5a5a). Desenha o fundo antes do conteúdo. */
static int box_begin(float x, float y, float w) {
    Op *o = rect(x, y, w, 10, C_PANEL);
    (void)o;
    return nops - 1;
}

static float box_end(int bg, float x, float y, float w, float bottom) {
    float h = bottom - y + 20;
    ops[bg].h = h;
    frame(x, y, w, h, C_BORDER, C_BORDER, -1);
    halo(x, y, w, h, 0, .75f, .75f, -1);
    return h;
}

static void section_title(const char *s, float x, float *y, float w) {
    *y += heading(s, x, *y, w, 2, C_WHITE, 0, 0, 1, 24) + 10;
    rect(x, *y, w, 4, C_LINE);
    *y += 4 + 12;
}

/* Imagem de um post: primeiro o cache do store.json (PNG/JPG baixado), senão a .wrt do VPK. */
static vita2d_texture *load_image(const char *rel, const char *slug, const char *wrt, int pixel) {
    vita2d_texture *t = NULL;
    if (rel && rel[0]) {
        char path[256];
        store_cache_path(rel, path, sizeof(path));
        if (fs_exists(path)) {
            size_t n = strlen(path);
            t = (n > 4 && !strcasecmp(path + n - 4, ".png")) ? vita2d_load_PNG_file(path) : vita2d_load_JPEG_file(path);
            if (t) {
                if (pixel) vita2d_texture_set_filters(t, SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_POINT);
                else vita2d_texture_set_filters(t, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);
            }
        }
    }
    if (!t) {
        char path[160];
        snprintf(path, sizeof(path), "app0:data/posts/%s/%s", slug, wrt);
        t = tex_load(path, pixel);
    }
    return t;
}

static void free_shots(void) {
    vita2d_wait_rendering_done();
    for (int i = 0; i < 16; i++)
        if (shot_tex[i]) {
            vita2d_free_texture(shot_tex[i]);
            shot_tex[i] = NULL;
        }
    shot_count = 0;
}

static void load_shots(int pi) {
    free_shots();
    Post *p = &g_posts[pi];
    for (int i = 0; i < p->shots && i < 16; i++) {
        char wrt[32];
        snprintf(wrt, sizeof(wrt), "shot%d.wrt", i);
        shot_tex[i] = load_image(p->shot_url[i], p->slug, wrt, p->pixel);
        if (shot_tex[i]) shot_count = i + 1;
    }
}

static void build_post(int pi) {
    layout_begin();
    Post *p = &g_posts[pi];
    PostText *t = &p->t[lang];
    float y = page_bar(header() + 24, A_BACK, TXT(S_CAT0 + p->cat), t->title) + 32;
    y += heading(t->title, 24, y, 912, 4, C_WHITE, C_SHADOW, 1, 0, 48) + 8;
    char line[512];
    snprintf(line, sizeof(line), "%s \xb7 %s", t->tag, t->status);
    upper_latin1(line);
    y += para(line, 24, y, 912, C_MUTED, 1, 2, 0) + 16;
    rect(24, y, 912, 4, C_LINE);
    y += 4 + 28;

    pop_range(0, view_t0 + .05);
    /* coluna principal */
    int first_left = nops;
    float lx = 24, lw = 560, ly = y;
    if (cover_tex[pi]) {
        float ch = (lw - 8) * 9 / 16;
        rect(lx, ly, lw, ch + 8, C_WHITE);
        Op *o = op(OP_TEX, lx + 4, ly + 4, lw - 8, ch);
        o->tex = cover_tex[pi];
        halo(lx, ly, lw, ch + 8, 0, .75f, .75f, -1);
        ly += ch + 8 + 28;
    }
    if (t->about[0]) ly += para(t->about, lx, ly, lw, RGBA8(0xE0, 0xE0, 0xE0, 0xFF), 0, 64, 0) + 28;
    for (int b = 0; b < t->nblocks; b++) {
        Block *k = &t->blocks[b];
        if (k->type == BLK_SEC) {
            if (b) ly += 16;
            section_title(k->text, lx, &ly, lw);
        } else if (k->type == BLK_PARA) {
            ly += para(k->text, lx, ly, lw, C_TEXT, 0, 64, 0) + 8;
        } else {
            rect(lx + 4, ly + 10, 6, 6, C_TEXT); /* list-style: square */
            ly += para(k->text, lx + 22, ly, lw - 22, C_TEXT, 0, 64, 0) + 6;
        }
    }
    if (shot_count) {
        ly += 22;
        section_title(TXT(S_SHOTS), lx, &ly, lw);
        float sw = (lw - 16) / 2, sh = (sw - 8) * 9 / 16 + 8;
        for (int i = 0; i < shot_count; i++) {
            float x = lx + (i % 2) * (sw + 16), yy = ly + (i / 2) * (sh + 16);
            int it = item(x, yy, sw, sh, A_SHOT, i, NULL);
            rect(x, yy, sw, sh, RGBA8(0, 0, 0, 0xFF));
            Op *o = op(OP_TEX, x + 4, yy + 4, sw - 8, sh - 8);
            o->tex = shot_tex[i];
            frame(x, yy, sw, sh, C_BORDER, C_WHITE, it);
            halo(x, yy, sw, sh, 0, 0, 1, it);
        }
        ly += ((shot_count + 1) / 2) * (sh + 16) - 16;
    }
    if (p->video || p->video_file) {
        ly += 28;
        section_title(TXT(S_VIDEO), lx, &ly, lw);
        float vh = (lw - 8) * 9 / 16 + 8;
        int it = p->video_file ? item(lx, ly, lw, vh, A_VIDEO, pi, NULL) : item(lx, ly, lw, vh, A_URL, 0, p->video);
        rect(lx, ly, lw, vh, RGBA8(0, 0, 0, 0xFF));
        if (cover_tex[pi]) {
            Op *o = op(OP_TEX, lx + 4, ly + 4, lw - 8, vh - 8);
            o->tex = cover_tex[pi];
            o->c = o->cf = RGBA8(0xFF, 0xFF, 0xFF, 0x73); /* opacity .45 */
        }
        frame(lx, ly, lw, vh, C_WHITE, C_WHITE, it);
        op(OP_PLAY, lx + lw / 2 - 32, ly + vh / 2 - 52, 64, 64);
        const char *w = keep_upper(TXT(p->video_file ? S_WATCH_APP : S_WATCH));
        float ww = title_width(w, 2);
        title(w, lx + lw / 2 - ww / 2 + 4, ly + vh / 2 + 32 + 4, 2, RGBA8(0, 0, 0, 0xFF), 0);
        title(w, lx + lw / 2 - ww / 2, ly + vh / 2 + 32, 2, C_WHITE, 0);
        halo(lx, ly, lw, vh, 0, 0, 1, it);
        ly += vh + 12;
        ly += para(TXT(p->video_file ? S_VIDEO_NOTE_APP : S_VIDEO_NOTE), lx, ly, lw, C_MUTED, 0, 3, 0);
    }

    pop_range(first_left, view_t0 + .15);
    /* coluna lateral: cada caixa entra um pouco depois da outra */
    double box_pop = view_t0 + .25;
    float rx = 616, rw = 320, ry = y, in = 20;
    if (p->ndls) {
        int bg = box_begin(rx, ry, rw);
        float yy = ry + in, iw = rw - 2 * in;
        yy += heading(TXT(S_DOWNLOADS), rx + in, yy, iw, 2, C_WHITE, 0, 0, 1, 24) + 16;
        int plug = -1;
        for (int i = 0; i < p->ndls; i++) {
            Download *d = &p->dls[i];
            DlStatus st;
            dl_status(p, i, &st);
            if (i) {
                rect(rx + in, yy, iw, 4, C_LINE);
                yy += 4 + 14;
            }
            yy += para(d->name, rx + in, yy, iw, C_WHITE, 0, 3, 1);
            char meta[96], sz[24];
            fmt_size(d->size, sz, sizeof(sz));
            snprintf(meta, sizeof(meta), "%s%s%s%s", d->version[0] ? "v" : "", d->version, d->version[0] && sz[0] ? " \xb7 " : "", sz);
            if (meta[0]) yy += para(meta, rx + in, yy, iw, C_MUTED, 0, 1, 0);
            if (st.installed) {
                /* Instalado v1.3 · Atualizado  /  Instalado v1.3 · Nova versão: v1.4 */
                char iv[128];
                if (st.inst_ver[0]) snprintf(iv, sizeof(iv), "%s%s", TXT(S_INSTALLED_V), st.inst_ver);
                else snprintf(iv, sizeof(iv), "%s", TXT(S_ST_INSTALLED));
                if (st.update) snprintf(iv + strlen(iv), sizeof(iv) - strlen(iv), " \xb7 %s%s", TXT(S_ST_NEWVER), d->version);
                else if (st.inst_ver[0] && d->version[0]) snprintf(iv + strlen(iv), sizeof(iv) - strlen(iv), " \xb7 %s", TXT(S_ST_UPTODATE));
                yy += para(iv, rx + in, yy, iw, st.update ? C_WHITE : C_MUTED, 0, 2, st.update);
            }
            yy += 10;
            const char *label = st.update ? TXT(S_DL_UPDATE)
                              : d->type == DL_VPK ? (st.installed ? TXT(S_DL_REINSTALL) : TXT(S_DL_INSTALL))
                              : d->type == DL_TRANS ? (st.installed ? TXT(S_DL_TRANS_RE) : TXT(S_DL_TRANS))
                              : d->type == DL_DATA ? TXT(S_DL_DATA)
                                                    : TXT(S_DL_GET);
            if (st.installed && st.tid[0]) { /* baixado e instalado: Abrir primeiro */
                int ob = button(TXT(S_OPEN_APP), rx + in, yy, iw, !st.update, A_LAUNCH, i, NULL, -1);
                if (ob >= 0) yy += items[ob].h + 12;
            }
            int it = button(label, rx + in, yy, iw, !(st.installed && st.tid[0]) || st.update, A_DL, i, NULL, -1);
            if (it >= 0) {
                Op *o = op(OP_DLJOB, items[it].x, items[it].y, items[it].w, items[it].h);
                o->scale = i;
                o->s = p->slug;
                o->item = it;
                yy += items[it].h + 14;
            }
            if (d->type == DL_VPK && d->needs_data) {
                op(OP_WARN, rx + in, yy + 2, 20, 20);
                yy += para(TXT(S_DATA_WARN), rx + in + 28, yy, iw - 28, C_TEXT, 0, 6, 0) + 10;
                if (d->instr[0]) {
                    int b = button(TXT(S_INSTR), rx + in, yy, iw, 0, A_URL, 0, d->instr, -1);
                    if (b >= 0) yy += items[b].h + 14;
                }
            } else if (d->type == DL_TRANS) {
                char to[160];
                snprintf(to, sizeof(to), "%s ux0:rePatch/%s%s", TXT(S_TRANS_TO), d->ntids ? d->tids[0] : "?", d->ntids > 1 ? " (+)" : "");
                yy += para(to, rx + in, yy, iw, C_MUTED, 0, 3, 0) + 6;
                if (plug < 0) plug = repatch_plugin();
                if (!plug) {
                    op(OP_WARN, rx + in, yy + 2, 20, 20);
                    yy += para(TXT(S_REPATCH_NO), rx + in + 28, yy, iw - 28, C_TEXT, 0, 4, 0) + 10;
                } else {
                    yy += para(TXT(S_REPATCH_OK), rx + in, yy, iw, C_MUTED, 0, 2, 0) + 10;
                }
            } else if (d->type == DL_DATA) {
                yy += para(TXT(S_DATA_TO), rx + in, yy, iw, C_MUTED, 0, 3, 0) + 10;
            } else if (d->type == DL_PLUGIN || d->type == DL_FILE) {
                yy += para(TXT(S_PLUGIN_NOTE), rx + in, yy, iw, C_TEXT, 0, 4, 0) + 10;
                if (d->instr[0]) {
                    int b = button(TXT(S_INSTR), rx + in, yy, iw, 0, A_URL, 0, d->instr, -1);
                    if (b >= 0) yy += items[b].h + 14;
                }
            }
        }
        ry += box_end(bg, rx, ry, rw, yy - 14) + 36;
        pop_range(bg, box_pop);
        box_pop += .1;
    }
    if (t->ninfo) {
        int bg = box_begin(rx, ry, rw);
        float yy = ry + in;
        yy += heading(TXT(S_INFO), rx + in, yy, rw - 2 * in, 2, C_WHITE, 0, 0, 1, 24) + 16;
        for (int i = 0; i < t->ninfo; i++) {
            float kw = body_width(t->info[i].a), full = rw - 2 * in;
            if (kw > 120) kw = 120;
            float vw = full - kw - 16;
            /* valor com palavra comprida demais para o lado da chave: vai para a linha de baixo */
            int fits = 1;
            const char *v = t->info[i].b;
            while (*v) {
                char word[256];
                int n = 0;
                while (v[n] && v[n] != ' ' && n < 255) n++;
                memcpy(word, v, n);
                word[n] = 0;
                if (body_width(word) + 2 > vw) fits = 0;
                v += n;
                while (*v == ' ') v++;
            }
            if (fits) {
                float a = para(t->info[i].a, rx + in, yy, kw + 4, C_MUTED, 0, 4, 0);
                float b = para(t->info[i].b, rx + in + kw + 16, yy, vw, C_WHITE, 2, 6, 1);
                yy += (a > b ? a : b) + 8;
            } else {
                yy += para(t->info[i].a, rx + in, yy, full, C_MUTED, 0, 4, 0);
                yy += para(t->info[i].b, rx + in, yy, full, C_WHITE, 2, 6, 1) + 8;
            }
            rect(rx + in, yy, rw - 2 * in, 4, C_LINE);
            yy += 4 + 8;
        }
        ry += box_end(bg, rx, ry, rw, yy - 8) + 36;
        pop_range(bg, box_pop);
        box_pop += .1;
    }
    if (p->progress >= 0 || t->nbars) {
        int bg = box_begin(rx, ry, rw);
        float yy = ry + in;
        yy += heading(t->bars_title ? t->bars_title : TXT(S_PROGRESS), rx + in, yy, rw - 2 * in, 2, C_WHITE, 0, 0, 1, 24) + 16;
        for (int i = -1; i < t->nbars; i++) {
            if (i < 0 && p->progress < 0) continue;
            const char *label = i < 0 ? TXT(S_TOTAL) : t->bars[i].a;
            int pct = i < 0 ? p->progress : t->bars[i].pct;
            char v[64];
            if (i >= 0 && t->bars[i].c[0]) snprintf(v, sizeof(v), "%s", t->bars[i].c);
            else snprintf(v, sizeof(v), "%d%%", pct);
            float vw = title_width(v, 2);
            float lh = para(label, rx + in, yy, rw - 2 * in - vw - 12, C_WHITE, 0, 3, 0);
            title(keep(v), rx + rw - in - vw, yy + 5, 2, C_WHITE, 0);
            yy += lh + 8;
            Op *o = op(OP_BAR, rx + in + 4, yy + 4, rw - 2 * in - 8, 24);
            o->scale = pct;
            yy += 32 + 12;
        }
        ry += box_end(bg, rx, ry, rw, yy - 12) + 36;
        pop_range(bg, box_pop);
        box_pop += .1;
    }
    /* Downloads (links do site) só quando o post não tem downloads do app, que já fazem o mesmo aqui dentro;
     * depois a caixa Source e o Clock recomendado, na ordem do site */
    for (int b = 0; b < 2; b++) {
        int n = b ? t->nsrcs : (p->ndls ? 0 : t->nlinks);
        if (!n) continue;
        Pair *lk = b ? t->srcs : t->links;
        int bg = box_begin(rx, ry, rw);
        float yy = ry + in;
        yy += heading(TXT(b ? S_SOURCE : S_LINKS), rx + in, yy, rw - 2 * in, 2, C_WHITE, 0, 0, 1, 24) + 16;
        for (int i = 0; i < n; i++) {
            int it = button(lk[i].a, rx + in, yy, rw - 2 * in, i == 0 && !b, A_URL, 0, lk[i].b, -1);
            yy += (it >= 0 ? items[it].h : 48) + 12;
        }
        ry += box_end(bg, rx, ry, rw, yy - 12) + 36;
        pop_range(bg, box_pop);
        box_pop += .1;
    }
    if (p->trophies) { /* troféus nativos: abaixo da caixa Source, só o ícone e o texto */
        const char *tl = keep_upper(TXT(S_TROPHIES));
        float tw = title_width(tl, 2), bw = 30 + 14 + tw, bx = rx + rw / 2 - bw / 2;
        int f0 = nops;
        Op *o = op(OP_TROPHY, bx, ry, 30, 30);
        o->scale = 3;
        title(tl, bx + 30 + 14, ry + 7, 2, C_GOLD, 0);
        pop_range(f0, box_pop);
        box_pop += .1;
        ry += 30 + 36;
    }
    if (t->nclock) {
        int bg = box_begin(rx, ry, rw);
        float yy = ry + in, full = rw - 2 * in;
        yy += heading(TXT(S_CLOCK), rx + in, yy, full, 2, C_WHITE, 0, 0, 1, 24) + 16;
        for (int i = 0; i < t->nclock; i++) {
            yy += para(t->clock[i].a, rx + in, yy, full, C_MUTED, 0, 3, 0);
            yy += para(t->clock[i].b, rx + in, yy, full, C_WHITE, 2, 4, 1) + 8;
            rect(rx + in, yy, full, 4, C_LINE);
            yy += 4 + 8;
        }
        ry += box_end(bg, rx, ry, rw, yy - 8) + 36;
        pop_range(bg, box_pop);
        box_pop += .1;
    }
    float end = ly > ry ? ly : ry;
    page_h = footer(end);
}

static const char *job_state_text(const Job *j) {
    switch (j->state) {
    case J_QUEUED: return TXT(S_QUEUED);
    case J_DOWNLOAD: return TXT(S_DOWNLOADING);
    case J_EXTRACT: return j->phase ? TXT(S_INSTALLING) : TXT(S_EXTRACTING);
    case J_INSTALL: return TXT(S_INSTALLING);
    case J_DONE: return TXT(S_DONE);
    case J_CANCELED: return TXT(S_CANCELED);
    default: return TXT(S_ERROR);
    }
}

static void build_dls(void) {
    layout_begin();
    float y = page_bar(header() + 24, A_BACK, TXT(S_DOWNLOADS), NULL) + 32;
    char h[64];
    snprintf(h, sizeof(h), "%s", TXT(S_DOWNLOADS));
    upper_latin1(h);
    title(keep(h), 24, y, 4, C_WHITE, C_SHADOW);
    int n = jobs_count(), finished = 0;
    for (int i = 0; i < n; i++) {
        Job *j = jobs_get(i);
        if (j && j->state >= J_DONE) finished = 1;
    }
    if (finished) { /* "Limpar concluídos" na linha do título */
        float w = title_width(keep_upper(TXT(S_CLEAR)), 2) + 48;
        button(TXT(S_CLEAR), 936 - w, y - 8, w, 0, A_CLEAR, 0, NULL, -1);
    }
    y += 56;
    rect(24, y, 912, 4, C_LINE);
    y += 4 + 32;
    if (!n && !reg_count()) y += para(TXT(S_NO_JOBS), 24, y, 912, C_MUTED, 1, 3, 0) + 24;
    if (n) {
        y += heading(TXT(S_QUEUE), 24, y, 912, 2, C_SOFT, 0, 0, 1, 24) + 10;
        rect(24, y, 912, 4, C_LINE);
        y += 4 + 20;
    }
    for (int i = n - 1; i >= 0; i--) { /* mais recente primeiro */
        Job *j = jobs_get(i);
        if (!j) continue;
        float rh = 112;
        int first = nops;
        rect(24, y, 912, rh, C_PANEL);
        frame(24, y, 912, rh, C_BORDER, C_BORDER, -1);
        char t[128];
        snprintf(t, sizeof(t), "%s%s%s", j->name, j->version[0] ? " v" : "", j->version);
        char *lines[2];
        static char buf[256];
        int nl = wrap_title(t, 2, 560, lines, 1, buf, sizeof(buf));
        if (nl) title(keep(lines[0]), 48, y + 22, 2, C_WHITE, 0);
        Op *o = op(OP_JOBROW, 48, y + 54, 560, 40);
        o->scale = j->id;
        if (j->state < J_DONE) button(TXT(S_CANCEL), 936 - 24 - 220, y + 32, 220, 0, A_CANCEL, j->id, NULL, -1);
        pop_range(first, view_t0 + .08 + (n - 1 - i) * .05);
        y += rh + 20;
    }
    /* o que já foi instalado pelo app, com o estado de cada um */
    int shown = 0;
    for (int r = reg_count() - 1; r >= 0; r--) {
        RegEntry *e = reg_get(r);
        int pi = -1;
        for (int i = 0; i < g_nposts; i++)
            if (!strcmp(g_posts[i].slug, e->slug)) pi = i;
        if (pi < 0 || e->dl >= g_posts[pi].ndls) continue;
        DlStatus st;
        dl_status(&g_posts[pi], e->dl, &st);
        if (!st.installed) continue;
        if (!shown++) {
            y += 20;
            y += heading(TXT(S_INSTALLED_LIST), 24, y, 912, 2, C_SOFT, 0, 0, 1, 24) + 10;
            rect(24, y, 912, 4, C_LINE);
            y += 4 + 20;
        }
        const Download *d = &g_posts[pi].dls[e->dl];
        float rh = 96;
        int first = nops;
        rect(24, y, 912, rh, C_PANEL);
        frame(24, y, 912, rh, C_BORDER, C_BORDER, -1);
        char *lines[2];
        static char buf2[256];
        if (wrap_title(e->name[0] ? e->name : d->name, 2, 560, lines, 1, buf2, sizeof(buf2))) title(keep(lines[0]), 48, y + 20, 2, C_WHITE, 0);
        char s[192];
        snprintf(s, sizeof(s), "%s%s", TXT(S_INSTALLED_V), st.inst_ver[0] ? st.inst_ver : "?");
        if (st.update) snprintf(s + strlen(s), sizeof(s) - strlen(s), " \xb7 %s%s", TXT(S_ST_NEWVER), d->version);
        else snprintf(s + strlen(s), sizeof(s) - strlen(s), " \xb7 %s", TXT(S_ST_UPTODATE));
        snprintf(s + strlen(s), sizeof(s) - strlen(s), " \xb7 %s", e->path);
        para(s, 48, y + 50, 560, st.update ? C_WHITE : C_MUTED, 0, 1, st.update);
        if (st.update) button(TXT(S_DL_UPDATE), 936 - 24 - 220, y + 24, 220, 1, A_REGUPD, r, NULL, -1);
        else if (st.tid[0]) {
            static char tids[MAX_REG][16]; /* o botão guarda o ponteiro */
            snprintf(tids[r], sizeof(tids[r]), "%s", st.tid);
            button(TXT(S_OPEN_APP), 936 - 24 - 220, y + 24, 220, 1, A_LAUNCH, -1, tids[r], -1);
        } else button(TXT(S_OPEN_PROJECT), 936 - 24 - 220, y + 24, 220, 0, A_POST, pi, NULL, -1);
        pop_range(first, view_t0 + .15 + shown * .05);
        y += rh + 20;
    }
    page_h = footer(y);
}

/* ---------------------------------------------------------------- pesquisa */

/* minúsculas e sem acento (Latin-1) para comparar */
static void fold(const char *in, char *out, int n) {
    static const char *from = "\xe0\xe1\xe2\xe3\xe4\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf1\xf2\xf3\xf4\xf5\xf6\xf9\xfa\xfb\xfc";
    static const char *to = "aaaaaceeeeiiiinooooouuuu";
    int o = 0;
    for (const unsigned char *p = (const unsigned char *)in; *p && o < n - 1; p++) {
        unsigned c = *p;
        if (c >= 'A' && c <= 'Z') c += 32;
        else if (c >= 0xC0 && c <= 0xDE && c != 0xD7) c += 32;
        const char *f = strchr(from, (int)c);
        out[o++] = (char)(c >= 0xE0 && f ? to[f - from] : c);
    }
    out[o] = 0;
}

static int post_matches(const Post *p, const char *query) {
    static char hay[4096], q[256], w[64];
    hay[0] = 0;
    for (int l = 0; l < 2; l++) {
        const PostText *t = &p->t[l];
        snprintf(hay + strlen(hay), sizeof(hay) - strlen(hay), " %s %s %s %s %s", t->title, t->tag, t->desc, t->status, UI[l][S_CAT0 + p->cat]);
    }
    snprintf(hay + strlen(hay), sizeof(hay) - strlen(hay), " %s", p->slug);
    fold(hay, hay, sizeof(hay));
    fold(query, q, sizeof(q));
    int any = 0;
    for (char *s = q; *s;) { /* todas as palavras precisam aparecer */
        while (*s == ' ') s++;
        int n = 0;
        while (s[n] && s[n] != ' ' && n < 63) n++;
        if (!n) break;
        memcpy(w, s, n);
        w[n] = 0;
        if (!strstr(hay, w)) return 0;
        any = 1;
        s += n;
    }
    return any;
}

static void build_search(void) {
    layout_begin();
    float y = page_bar(header() + 24, A_BACK, TXT(S_SEARCH), NULL) + 32;
    char h[64];
    snprintf(h, sizeof(h), "%s", TXT(S_SEARCH));
    upper_latin1(h);
    title(keep(h), 24, y, 4, C_WHITE, C_SHADOW);
    y += 56;
    rect(24, y, 912, 4, C_LINE);
    y += 4 + 28;
    /* campo de busca: abre o teclado do Vita */
    int it = item(24, y, 912, 64, A_SEARCHBOX, 0, NULL);
    int first = nops;
    rect(24, y, 912, 64, C_DARK);
    frame(24, y, 912, 64, C_BTNBRD, C_WHITE, it);
    op(OP_ICON, 24 + 20, y + 16, 32, 32)->scale = A_SEARCH;
    if (search_q[0]) body(keep(search_q), 24 + 68, y + 19, C_WHITE)->bold = 1;
    else body(TXT(S_TIP_SEARCH), 24 + 68, y + 19, C_MUTED);
    for (int i = first; i < nops; i++) ops[i].item = it;
    halo(24, y, 912, 64, 0, 0, 1, it);
    y += 64 + 28;
    if (!search_q[0]) {
        y += para(TXT(S_SEARCH_EMPTY), 24, y, 912, C_MUTED, 1, 2, 0);
    } else {
        int ids[MAX_POSTS], n = 0;
        for (int i = 0; i < g_nposts; i++)
            if (post_matches(&g_posts[i], search_q)) ids[n++] = i;
        char c[192];
        snprintf(c, sizeof(c), "%d %s \"%s\"", n, TXT(S_RESULTS), search_q);
        if (!n) snprintf(c, sizeof(c), "%s", TXT(S_NO_RESULTS));
        y += para(c, 24, y, 912, C_SOFT, 1, 2, 0) + 24;
        if (n) y += grid(ids, n, y, view_t0 + .1, .05);
    }
    page_h = footer(y);
}

static void rebuild(void) {
    if (view == V_HOME) build_home();
    else if (view == V_CAT) build_cat(view_cat);
    else if (view == V_DLS) build_dls();
    else if (view == V_SEARCH) build_search();
    else build_post(view_post);
    if (focus >= nitems) focus = nitems - 1;
}

/* ================================================================ desenho */

/* wr-pop: translateY(16px) e opacidade 0 -> normal em .32s, 4 passos */
static int pop_state(double pop, float *dy, float *alpha) {
    *dy = 0;
    *alpha = 1;
    if (pop < 0) return 1;
    double e = now_s - pop;
    if (e < 0) return 0;
    if (e >= .32) return 1;
    int st = (int)(e / .08) + 1; /* steps(4, end) */
    if (st > 4) st = 4;
    float k = st / 4.f;
    *dy = 16 * (1 - k);
    *alpha = k;
    return 1;
}

static void draw_play(float x, float y) {
    /* o mesmo ícone 16x16 do site, ampliado 4x */
    vita2d_draw_rectangle(x, y, 64, 64, C_DARK);
    vita2d_draw_rectangle(x + 4, y + 4, 56, 56, C_WHITE);
    vita2d_draw_rectangle(x + 8, y + 8, 48, 48, C_DARK);
    vita2d_draw_rectangle(x + 20, y + 16, 8, 32, C_WHITE);
    vita2d_draw_rectangle(x + 28, y + 20, 8, 24, C_WHITE);
    vita2d_draw_rectangle(x + 36, y + 24, 8, 16, C_WHITE);
    vita2d_draw_rectangle(x + 44, y + 28, 4, 8, C_WHITE);
}

static void draw_bar(float x, float y, float w, int pct) {
    /* .wr-pb: moldura branca 4px, fundo escuro, segmentos de 8px a cada 12px */
    frame_rect(x - 4, y - 4, w + 8, 32, 4, C_WHITE);
    vita2d_draw_rectangle(x, y, w, 24, C_DARK);
    float iw = w - 8;
    for (float sx = 0; sx < iw; sx += 12) {
        float sw = sx + 8 > iw ? iw - sx : 8;
        vita2d_draw_rectangle(x + 4 + sx, y + 4, sw, 16, RGBA8(0x24, 0x24, 0x24, 0xFF));
    }
    /* wr-fill: 1,1 s em 12 passos, começando 0,35 s depois de abrir a tela */
    double e = now_s - view_t0 - .35;
    float k = e <= 0 ? 0 : e >= 1.1 ? 1 : ((int)(e / (1.1 / 12)) + 1) / 12.f;
    float fw = iw * pct / 100.f * (k > 1 ? 1 : k);
    for (float sx = 0; sx < fw; sx += 12) {
        float sw = sx + 8 > fw ? fw - sx : 8;
        vita2d_draw_rectangle(x + 4 + sx, y + 4, sw, 16, C_WHITE);
    }
}

static void draw_logo(float x, float y) {
    /* latido: a pata pula em 6 passos (como o wr-bark do site) e aparece o balão */
    double be = now_s - bark_t0;
    float pdy = 0, pdx = 0;
    if (be >= 0 && be < .6) {
        static const float dy[6] = {4, -12, 0, -6, 0, 0}, dx[6] = {0, -2, 0, 2, 0, 0};
        int st = (int)(be / .1);
        pdy = dy[st];
        pdx = dx[st];
    }
    paw_draw(x + pdx, y + pdy, 1, 1);
    float lx = LOGO_X - 24 + x, ly = y + 22;
    for (int i = 0; LOGO[i]; i++) {
        char ch[2] = {LOGO[i], 0};
        /* .wr-l:hover sobe 10px; aqui a onda passa pelas letras (triângulo) */
        double e = now_s - wave_t0 - i * .06, eh = i < 16 ? now_s - letter_hop[i] : 1;
        float up = ((e >= 0 && e < .14) || (eh >= 0 && eh < .14)) ? 8 : 0;
        float cx = lx + i * LOGO_STEP;
        title_draw(ch, cx + 6, ly + 6 - up, LOGO_SC, C_SH2);
        title_draw(ch, cx + 3, ly + 3 - up, LOGO_SC, C_SH1);
        title_draw(ch, cx, ly - up, LOGO_SC, C_WHITE);
    }
    if (be >= .05 && be < .95) {
        const char *b = TXT(S_BARK);
        float bw = title_width(b, 2) + 16, bx = x + 76, by = y - 4 - (be > .7 ? 4 : 0);
        vita2d_draw_rectangle(bx + 4, by + 4, bw, 28, C_SH2);
        vita2d_draw_rectangle(bx, by, bw, 28, C_DARK);
        frame_rect(bx, by, bw, 28, 4, C_WHITE);
        title_draw(b, bx + 8, by + 6, 2, C_WHITE);
    }
}

/* Botões do topo: ícones de pixel 8x8 (ampliados 4x), no estilo da pata */
static const char *ICON_SYNC[8] = {"..####..", ".#....##", "#....###", "#.......", "#......#", ".#....#.", "..####..", "........"};
static const char *ICON_DL[8] = {"...##...", "...##...", "...##...", ".######.", "..####..", "...##...", "#......#", "########"};
static const char *ICON_SND_ON[8] = {"...#....", "..##..#.", "####...#", "####.#.#", "####.#.#", "####...#", "..##..#.", "...#...."};
static const char *ICON_SND_OFF[8] = {"...#....", "..##....", "####.#.#", "####..#.", "####..#.", "####.#.#", "..##....", "...#...."};
static const char *ICON_GEAR[8] = {"..#..#..", ".######.", "###..###", ".#....#.", ".#....#.", "###..###", ".######.", "..#..#.."};
static const char *ICON_SEARCH[8] = {".####...", "#....#..", "#....#..", "#....#..", ".####...", "....##..", ".....##.", "......##"};

static void icon8(const char **ic, float x, float y, int s, unsigned c) {
    for (int r = 0; r < 8; r++)
        for (int k = 0; k < 8; k++)
            if (ic[r][k] == '#') vita2d_draw_rectangle(x + k * s, y + r * s, s, s, c);
}

static void draw_badge(int kind, float x, float y, int f, float al) {
    /* sem moldura nem halo: só o ícone (com sombra de 4 px, como o texto do site);
     * com foco ele fica branco e ganha um traço embaixo */
    unsigned W = col_alpha(C_WHITE, al);
    unsigned ic = f ? W : col_alpha(C_MUTED, al);
    float ix = x + (BADGE - 32) / 2, iy = y + (BADGE - 32) / 2 - 4;
    if (kind == A_SETTINGS || kind == A_SEARCH) {
        icon8(kind == A_SETTINGS ? ICON_GEAR : ICON_SEARCH, ix + 4, iy + 4, 4, col_alpha(C_SHADOW, al));
        icon8(kind == A_SETTINGS ? ICON_GEAR : ICON_SEARCH, ix, iy, 4, ic);
        return;
    }
    if (kind == A_SYNC || kind == A_DLS) icon8(kind == A_SYNC ? ICON_SYNC : ICON_DL, ix + 4, iy + 4, 4, col_alpha(C_SHADOW, al));
    if (kind == A_SOUND) {
        const char **si = (audio_enabled() || audio_music()) ? ICON_SND_ON : ICON_SND_OFF;
        icon8(si, ix + 4, iy + 4, 4, col_alpha(C_SHADOW, al));
        icon8(si, ix, iy, 4, snd_menu ? W : ic);
    } else if (kind == A_LANG) {
        const char *l = lang == LANG_EN ? "EN" : "PT";
        title_draw(l, x + (BADGE - title_width(l, 2)) / 2, y + 18, 2, ic);
    } else if (kind == A_SYNC && (sync_state() == SYNC_RUNNING || now_s - sync_done_t < 1.2)) {
        /* sincronizando: o percentual no lugar do ícone ("Sincron..." fica no nome embaixo) */
        char pc[8];
        snprintf(pc, sizeof(pc), "%d%%", sync_state() == SYNC_RUNNING ? (int)(sync_progress() * 100) : 100);
        float tw = title_width(pc, 2);
        title_draw(pc, x + (BADGE - tw) / 2 + 2, iy + 12 + 2, 2, col_alpha(C_SHADOW, al));
        title_draw(pc, x + (BADGE - tw) / 2, iy + 12, 2, W);
    } else if (kind == A_SYNC) {
        if (sync_state() == SYNC_RUNNING) {
            /* gira em 4 passos enquanto sincroniza */
            int st = (int)(now_s * 8) % 4;
            for (int r = 0; r < 8; r++)
                for (int k = 0; k < 8; k++) {
                    int rr = r, kk = k;
                    if (st == 1) { rr = k; kk = 7 - r; }
                    else if (st == 2) { rr = 7 - r; kk = 7 - k; }
                    else if (st == 3) { rr = 7 - k; kk = r; }
                    if (ICON_SYNC[rr][kk] == '#') vita2d_draw_rectangle(ix + k * 4, iy + r * 4, 4, 4, ic);
                }
        } else {
            icon8(ICON_SYNC, ix, iy, 4, ic);
        }
    } else if (kind == A_DLS) {
        icon8(ICON_DL, ix, iy, 4, ic);
        int n = jobs_active();
        if (n) {
            Job *j = NULL;
            for (int i = 0; i < jobs_count(); i++) {
                Job *k = jobs_get(i);
                if (k && k->state > J_QUEUED && k->state < J_DONE) j = k;
            }
            /* barrinha do download atual na borda de baixo e a contagem no canto */
            if (j && j->pr.total > 0) {
                float k = (float)j->pr.done / j->pr.total;
                if (!f) {
                    vita2d_draw_rectangle(ix, y + BADGE - 6, 32, 4, col_alpha(C_LINE, al));
                    vita2d_draw_rectangle(ix, y + BADGE - 6, 32 * (k > 1 ? 1 : k), 4, W);
                }
            }
            char c[4];
            snprintf(c, sizeof(c), "%d", n > 9 ? 9 : n);
            vita2d_draw_rectangle(x + BADGE - 18, y, 18, 18, W);
            title_draw(c, x + BADGE - 13, y + 5, 1, col_alpha(C_DARK, al));
        }
    }
}

/* Texto à esquerda dos botões: o nome do botão com foco, ou o estado da sincronização */
static void draw_header_label(int act, float cx, float y, int f, float al) {
    /* O nome aparece apenas no botão selecionado; os ícones sem foco ficam
     * limpos e ocupam menos atenção no topo. */
    static int was_running;
    if (act == A_SYNC) {
        int running = sync_state() == SYNC_RUNNING;
        if (was_running && !running) sync_done_t = now_s;
        was_running = running;
    }
    if (!f) return;
    char t[48];
    hdr_label(act, t, sizeof(t));
    /* Press Start 8 px: cabe ao lado do logo e continua nítido no Vita */
    title_draw(t, cx - title_width(t, 1) / 2, y + 6, 1, col_alpha(f || (act == A_SOUND && snd_menu) ? C_WHITE : C_MUTED, al));
}

/* ---------------------------------------------------------------- intro */

#define KICK_SCALE 4
#define KICK_LINE 56

static int kicker_layout(char **lines, char *buf, int bufsz) {
    return wrap_title(TXT(S_KICKER), KICK_SCALE, 880, lines, 6, buf, bufsz);
}

static float kicker_top(int n) { return (SCR_H - n * KICK_LINE) / 2.f - 40; }

/* Tempo em que a letra idx aparece (0,3 s + 45 ms por letra, o espaço também conta) */
static double kick_char_t(int idx) { return .3 + idx * .045; }

/* Tela preta com a pergunta digitada no centro. crt: 0 normal, >0 progresso da TV desligando. */
static void draw_title_screen(double tt, float crt) {
    char *lines[6];
    static char buf[512];
    int n = kicker_layout(lines, buf, sizeof(buf));
    float ty = kicker_top(n), bh = n * KICK_LINE;
    float sx = 1, sy = 1;
    if (crt >= 1) return;
    if (crt > 0 && crt <= .5f) {
        float q = crt / .5f;
        sx = 1 + .04f * q;
        sy = 1 - .96f * q;
    } else if (crt > .5f) {
        float q = (crt - .5f) / .49f;
        sx = 1.04f * (1 - q);
        sy = .04f;
    }
    float ccx = SCR_W / 2.f, ccy = ty + bh / 2;
    int idx = 0, typed_all = 1;
    float caret_x = 0, caret_y = 0;
    for (int l = 0; l < n; l++) {
        int lw = title_width(lines[l], KICK_SCALE);
        float lx = (SCR_W - lw) / 2.f;
        for (int i = 0; lines[l][i]; i++, idx++) {
            double ct = kick_char_t(idx);
            if (tt < ct) {
                typed_all = 0;
                goto done;
            }
            caret_x = lx + (i + 1) * 8 * KICK_SCALE;
            caret_y = ty + l * KICK_LINE;
            if (lines[l][i] == ' ') continue;
            double e = tt - ct;
            float dy = e >= .12 ? 0 : -8 * (1 - ((int)(e / .04) + 1) / 3.f); /* wr-chpop */
            char ch[2] = {lines[l][i], 0};
            float gx = lx + i * 8 * KICK_SCALE, gy = ty + l * KICK_LINE + dy;
            float fx = ccx + (gx - ccx) * sx, fy = ccy + (gy - ccy) * sy;
            title_draw_f(ch, fx + 4 * sx, fy + 4 * sy, KICK_SCALE * sx, KICK_SCALE * sy, C_SHADOW);
            title_draw_f(ch, fx, fy, KICK_SCALE * sx, KICK_SCALE * sy, C_WHITE);
        }
        idx++;
    }
done:
    if (crt > 0) {
        /* linha branca da TV desligando */
        if (crt > .2f) vita2d_draw_rectangle(ccx - 440 * sx, ccy - 2, 880 * sx, 4, col_alpha(C_WHITE, .9f));
        return;
    }
    /* cursor piscando enquanto espera */
    int vis = fmod(tt, .5) < .25;
    if (vis && caret_x > 0) vita2d_draw_rectangle(caret_x + 6, caret_y + 4, 16, 24, C_WHITE);
    if (!typed_all) return;
    /* convite: aparece depois da digitação e pisca (como o .wr-start do site) */
    double since = tt - kick_char_t(idx);
    if (since < .3) return;
    static char a[96];
    snprintf(a, sizeof(a), "%s", TXT(S_START));
    upper_latin1(a);
    float tw = title_width(a, 2) + 28, px = (SCR_W - tw) / 2, py = ty + bh + 56;
    float pa = since < .6 ? (since - .3) / .3 : 1;
    pa = ((int)(pa * 4)) / 4.f;
    int blink = fmod(tt, 1.0) < .7;
    unsigned c = col_alpha(blink ? C_WHITE : C_MUTED, pa);
    for (int i = 0; i < 4; i++) vita2d_draw_rectangle(px + i * 4, py + i * 2, 4, 16 - i * 4, c);
    title_draw(a, px + 28, py, 2, c);
    const char *b = TXT(S_TAP);
    body_draw(b, (SCR_W - body_width(b)) / 2, py + 32, col_alpha(C_MUTED, pa));
}

/* Capa do projeto atual: liga a TV grande no centro, depixeliza e encaixa no palco da página. */
static void draw_stage(const Op *o, float oy) {
    float x = o->x, y = o->y - oy, w = o->w, h = o->h;
    vita2d_texture *cov = cover_tex[feat];
    if (!started) return;
    /* duas linhas do tempo: a intro completa e a volta para o início (mais curta, sem deslizar) */
    double t = now_s - intro_t0, t_tv = T_TV, tv_step = .075, t_mos = T_MOS, mos_step = .13, t_halo = T_HALO, halo_d = .48;
    int mos_first = 0, intro = t < T_DONE, done;
    if (intro) {
        done = 0;
    } else {
        t = now_s - home_t0;
        t_tv = 0;
        tv_step = .0375;
        t_mos = .16;
        mos_step = .06;
        mos_first = 3;
        t_halo = .3;
        halo_d = .24;
        done = t >= .9;
    }
    if (t < t_tv) return;

    /* retângulo: grande no centro, depois desliza para o lugar em 8 passos (só na intro) */
    float bw = 720, bh = 405, bx = (SCR_W - bw) / 2, by = (SCR_H - bh) / 2;
    float k = (done || !intro) ? 1 : t < T_MOVE ? 0 : (int)((t - T_MOVE) / (.5 / 8) + 1) / 8.f;
    if (k > 1) k = 1;
    float ease = k * k * (3 - 2 * k);
    float rx = bx + (x - bx) * ease, ry = by + (y - by) * ease, rw = bw + (w - bw) * ease, rh = bh + (h - bh) * ease;

    /* TV ligando: linha horizontal cresce, depois abre na vertical, com brilho */
    float p = done ? 1 : (int)((t - t_tv) / tv_step + 1) / 8.f;
    if (p > 1) p = 1;
    float cw, ch, br;
    if (p < .4f) {
        float q = p / .4f;
        cw = rw * q;
        ch = 4;
        br = 4;
    } else {
        float q = (p - .4f) / .6f;
        cw = rw;
        ch = 4 + (rh - 4) * q;
        br = 4 - 3 * q;
    }
    float cx = rx + (rw - cw) / 2, cy = ry + (rh - ch) / 2;

    /* halo: inset -4 -> -40 -> -28, 4 passos (wr-halo) */
    double hp = done ? 1 : (t - t_halo) / halo_d;
    if (hp >= 0) {
        int st = hp >= 1 ? 4 : (int)(hp * 4) + 1;
        float kk = st / 4.f, in, al;
        if (kk <= .5f) {
            in = 4 + 36 * (kk / .5f);
            al = kk / .5f;
        } else {
            in = 40 - 12 * ((kk - .5f) / .5f);
            al = 1;
        }
        halo_draw(rx - in, ry - in, rw + 2 * in, rh + 2 * in, 1, al);
    }
    vita2d_enable_clipping();
    vita2d_set_clip_rectangle((int)cx, (int)cy, (int)(cx + cw), (int)(cy + ch));
    vita2d_draw_rectangle(rx, ry, rw, rh, RGBA8(0, 0, 0, 0xFF));
    int layer = -1;
    if (!done)
        for (int mm = mos_first; mm < 8; mm++)
            if (t < t_mos + (mm - mos_first) * mos_step) {
                layer = mm;
                break;
            }
    if (layer >= 0 && mos_tex[layer]) tex_draw(mos_tex[layer], rx, ry, rw, rh, C_WHITE);
    else if (cov) tex_draw(cov, rx, ry, rw, rh, C_WHITE);
    if (br > 1) vita2d_draw_rectangle(rx, ry, rw, rh, col_alpha(C_WHITE, (br - 1) / 3 * .8f));
    vita2d_disable_clipping();
}

/* ---------------------------------------------------------------- página */

static Job *job_by_id(int id) {
    for (int i = 0; i < jobs_count(); i++) {
        Job *j = jobs_get(i);
        if (j && j->id == id) return j;
    }
    return NULL;
}

static float job_frac(const Job *j) {
    if (j->state == J_DONE) return 1;
    if (j->pr.total <= 0) return 0;
    float k = (float)j->pr.done / j->pr.total;
    return k < 0 ? 0 : k > 1 ? 1 : k;
}

/* barra em blocos de 8 px a cada 12 px, como a .wr-pb do site */
static void seg_bar(float x, float y, float w, float h, float k, unsigned on, unsigned off) {
    for (float sx = 0; sx < w; sx += 12) {
        float sw = sx + 8 > w ? w - sx : 8;
        vita2d_draw_rectangle(x + sx, y, sw, h, sx + 8 <= w * k ? on : off);
    }
}

/* ícone de aviso em pixels (triângulo com "!") */
static void draw_warn(float x, float y, float al) {
    unsigned c = col_alpha(C_WHITE, al);
    for (int i = 0; i < 5; i++) vita2d_draw_rectangle(x + 8 - i * 2, y + i * 4, 4 + i * 4, 4, c);
    vita2d_draw_rectangle(x + 8, y + 6, 4, 6, col_alpha(C_DARK, al));
    vita2d_draw_rectangle(x + 8, y + 14, 4, 2, col_alpha(C_DARK, al));
}

/* por cima do botão de download: progresso do trabalho desse download */
static void draw_dljob(const Op *o, float x, float y, float al) {
    Job *j = jobs_find(o->s, o->scale);
    if (!j || j->state >= J_DONE) return;
    float w = o->w, h = o->h;
    vita2d_draw_rectangle(x, y, w, h, col_alpha(C_DARK, al));
    frame_rect(x, y, w, h, 4, col_alpha(C_WHITE, al));
    float k = job_frac(j);
    int busy = j->state == J_EXTRACT || j->state == J_INSTALL || j->state == J_QUEUED;
    if (busy && (j->state != J_EXTRACT || j->phase)) {
        /* instalando/na fila: listras andando */
        int off = (int)(now_s * 24) % 24;
        for (float sx = -24 + off; sx < w - 16; sx += 24)
            if (sx >= 0) vita2d_draw_rectangle(x + 8 + sx, y + h - 12, 12, 4, col_alpha(C_SOFT, al));
    } else {
        seg_bar(x + 8, y + h - 12, w - 16, 4, k, col_alpha(C_WHITE, al), col_alpha(RGBA8(0x24, 0x24, 0x24, 0xFF), al));
    }
    char t[64];
    if (j->state == J_DOWNLOAD || (j->state == J_EXTRACT && !j->phase)) snprintf(t, sizeof(t), "%s %d%%", job_state_text(j), (int)(k * 100));
    else snprintf(t, sizeof(t), "%s", job_state_text(j));
    upper_latin1(t);
    title_draw(t, x + (w - title_width(t, 2)) / 2, y + 12, 2, col_alpha(C_WHITE, al));
}

/* linha da tela de downloads: estado, barra, velocidade, mensagem */
static void draw_jobrow(const Op *o, float x, float y, float al) {
    Job *j = job_by_id(o->scale);
    if (!j) return;
    float k = job_frac(j);
    char t[256], a[24], b[24];
    fmt_size(j->pr.done, a, sizeof(a));
    fmt_size(j->pr.total, b, sizeof(b));
    if (j->state == J_DOWNLOAD) {
        char sp[24];
        fmt_size((long long)j->pr.speed, sp, sizeof(sp));
        char spd[40] = "";
        if (sp[0]) snprintf(spd, sizeof(spd), " \xb7 %s/s", sp);
        snprintf(t, sizeof(t), "%s %d%% \xb7 %s / %s%s", job_state_text(j), (int)(k * 100), a[0] ? a : "0 KB", b[0] ? b : "?", spd);
    } else if (j->state == J_DONE && j->msg[0] && strcmp(j->msg, "pronto")) {
        snprintf(t, sizeof(t), "%s \xb7 %s", job_state_text(j), j->msg);
    } else if (j->state == J_ERROR || j->state == J_CANCELED) {
        snprintf(t, sizeof(t), "%s%s%s", job_state_text(j), j->msg[0] ? ": " : "", j->msg);
    } else {
        snprintf(t, sizeof(t), "%s", job_state_text(j));
    }
    char *lines[2];
    static char buf[512];
    int n = wrap_body(t, (int)o->w, lines, 1, buf, sizeof(buf));
    if (n) body_draw(lines[0], x, y - 4, col_alpha(j->state == J_ERROR ? C_WHITE : C_SOFT, al));
    if (j->state < J_DONE || j->state == J_DONE) {
        int busy = (j->state == J_EXTRACT && j->phase) || j->state == J_INSTALL;
        vita2d_draw_rectangle(x, y + 26, o->w, 16, col_alpha(C_DARK, al));
        if (busy) {
            int off = (int)(now_s * 24) % 24;
            for (float sx = -24 + off; sx < o->w; sx += 24)
                if (sx >= 0) vita2d_draw_rectangle(x + sx, y + 30, 12, 8, col_alpha(C_SOFT, al));
        } else {
            seg_bar(x, y + 30, o->w, 8, k, col_alpha(C_WHITE, al), col_alpha(RGBA8(0x24, 0x24, 0x24, 0xFF), al));
        }
    }
}

static int focused(int it) { return it >= 0 && it == focus; }
static int last_focus = -2;
static double focus_t0 = -10;

static void draw_page(void) {
    float oy = cam;
    if (focus != last_focus) {
        last_focus = focus;
        focus_t0 = now_s;
    }
    for (int i = 0; i < nops; i++) {
        Op *o = &ops[i];
        float dy, al;
        if (!pop_state(o->pop, &dy, &al)) continue;
        float y = o->y - oy + dy, x = o->x;
        if (o->type != OP_HALO && (y > SCR_H || y + o->h + 64 < 0)) continue;
        if (o->type == OP_HALO && (y > SCR_H || y + o->h < 0)) continue;
        if (o->item >= 0 && o->item == pressed_item && now_s - pressed_t0 < .12) {
            x += 4;
            y += 4;
        }
        int f = focused(o->item);
        unsigned c = col_alpha(f ? o->cf : o->c, al);
        switch (o->type) {
        case OP_RECT: vita2d_draw_rectangle(x, y, o->w, o->h, c); break;
        case OP_FRAME: frame_rect(x, y, o->w, o->h, o->bold == 2 ? 2 : 4, c); break;
        case OP_TITLE:
            if (o->bold == 2) {
                title_draw(o->s, x + 8, y + 8, o->scale, col_alpha(RGBA8(0x1A, 0x1A, 0x1A, 0xFF), al));
                title_draw(o->s, x + 4, y + 4, o->scale, col_alpha(C_SHADOW, al));
            } else if (o->shadow) {
                title_draw(o->s, x + 4, y + 4, o->scale, col_alpha(o->shadow, al));
            }
            title_draw(o->s, x, y, o->scale, c);
            break;
        case OP_BODY:
            if (o->bold) body_draw_bold(o->s, x, y, c);
            else body_draw(o->s, x, y, c);
            break;
        case OP_TEX: tex_draw(o->tex, x, y, o->w, o->h, c); break;
        case OP_HALO: {
            float a = f ? o->af : o->a;
            float d = 0;
            if (f && now_s - focus_t0 < .12) {
                /* o halo "abre" ao receber o foco, em 3 passos */
                int st = (int)((now_s - focus_t0) / .04) + 1;
                d = 8 * (1 - st / 3.f);
                if (o->a <= 0) a *= st / 3.f;
            }
            if (a > 0) halo_draw(x + d, y + d, o->w - 2 * d, o->h - 2 * d, o->scale, a * al);
            break;
        }
        case OP_BAR: draw_bar(x, y, o->w, o->scale); break;
        case OP_STAGE: draw_stage(o, oy); break;
        case OP_LOGO: draw_logo(x, y); break;
        case OP_PLAY: draw_play(x, y); break;
        case OP_BADGE: draw_badge(o->scale, x, y, f, al); break;
        case OP_DLJOB: draw_dljob(o, x, y, al); break;
        case OP_JOBROW: draw_jobrow(o, x, y, al); break;
        case OP_WARN: draw_warn(x, y, al); break;
        case OP_HLABEL: draw_header_label(o->scale, x, y, f, al); break;
        case OP_ICON: icon8(o->scale == A_SEARCH ? ICON_SEARCH : ICON_GEAR, x, y, 4, col_alpha(C_SOFT, al)); break;
        case OP_TROPHY: { /* troféu 10x10 em pixel art */
            static const char *TR[10] = {".########.", "##.####.##", "#..####..#", "##.####.##", "..######..",
                                         "...####...", "....##....", "....##....", "..######..", "..######.."};
            float k = o->scale;
            for (int r = 0; r < 10; r++) /* sombra de 1 pixel, como o drop-shadow do site */
                for (int c = 0; c < 10; c++)
                    if (TR[r][c] == '#') vita2d_draw_rectangle(x + c * k + k, y + r * k + k, k, k, col_alpha(RGBA8(0, 0, 0, 0xFF), al));
            for (int r = 0; r < 10; r++)
                for (int c = 0; c < 10; c++)
                    if (TR[r][c] == '#') vita2d_draw_rectangle(x + c * k, y + r * k, k, k, col_alpha(C_GOLD, al));
            vita2d_draw_rectangle(x + 2 * k, y + 1 * k, k, 3 * k, col_alpha(RGBA8(0xFF, 0xF6, 0xC8, 0xFF), al));
            break;
        }
        case OP_TRI: /* seta para a esquerda (botão voltar) */
            for (int i = 0; i < 4; i++) vita2d_draw_rectangle(x + i * 4, y + 6 - i * 2, 4, 4 + i * 4, c);
            break;
        default: break;
        }
    }
}

/* ---------------------------------------------------------------- dicas */

static void btn_icon(int kind, float x, float y) {
    /* ✕ ○ △ □ em pixels, 16x16 */
    unsigned c = C_WHITE;
    if (kind == 0) {
        for (int i = 0; i < 4; i++) {
            vita2d_draw_rectangle(x + 2 + i * 3, y + 2 + i * 3, 3, 3, c);
            vita2d_draw_rectangle(x + 11 - i * 3, y + 2 + i * 3, 3, 3, c);
        }
    } else if (kind == 1) {
        vita2d_draw_rectangle(x + 5, y + 1, 6, 3, c);
        vita2d_draw_rectangle(x + 5, y + 12, 6, 3, c);
        vita2d_draw_rectangle(x + 1, y + 5, 3, 6, c);
        vita2d_draw_rectangle(x + 12, y + 5, 3, 6, c);
        vita2d_draw_rectangle(x + 3, y + 3, 3, 3, c);
        vita2d_draw_rectangle(x + 10, y + 3, 3, 3, c);
        vita2d_draw_rectangle(x + 3, y + 10, 3, 3, c);
        vita2d_draw_rectangle(x + 10, y + 10, 3, 3, c);
    } else if (kind == 2) {
        for (int i = 0; i < 5; i++) vita2d_draw_rectangle(x + 7 - i * 1.5f, y + 2 + i * 2.6f, 2 + i * 3, 3, c);
    } else if (kind == 3) {
        frame_rect(x + 2, y + 2, 12, 12, 3, c); /* □ */
    } else if (kind == 4) { /* direcional */
        vita2d_draw_rectangle(x + 6, y, 4, 16, c);
        vita2d_draw_rectangle(x, y + 6, 16, 4, c);
    }
}

static float hint_slide = 0;

static float hint(float x, int icon, const char *label, const char *key) {
    float y = SCR_H - 30 + hint_slide;
    if (key) {
        body_draw(key, x, y, C_MUTED);
        x += body_width(key) + 8;
    } else {
        btn_icon(icon, x, y + 5);
        x += 22;
    }
    body_draw(label, x, y, C_WHITE);
    return x + body_width(label) + 24;
}

static float draw_hints(void) {
    vita2d_draw_rectangle(0, SCR_H - 36 + hint_slide, SCR_W, 36, RGBA8(0x06, 0x06, 0x06, 0xE6));
    vita2d_draw_rectangle(0, SCR_H - 36 + hint_slide, SCR_W, 4, C_LINE);
    float x = 24;
    if (viewer >= 0) {
        x = hint(x, 1, TXT(S_H_CLOSE), NULL);
        x = hint(x, 0, TXT(S_H_NEXT), "L/R");
        return x;
    }
    if (snd_menu) {
        x = hint(x, 4, TXT(S_H_MOVE), NULL);
        x = hint(x, 0, TXT(S_H_SELECT), NULL);
        x = hint(x, 1, TXT(S_H_CLOSE), NULL);
        return x;
    }
    if (settings_open) {
        x = hint(x, 4, TXT(S_H_MOVE), NULL);
        x = hint(x, 0, TXT(S_H_SELECT), NULL);
        x = hint(x, 1, TXT(S_H_CLOSE), NULL);
        return x;
    }
    x = hint(x, 4, TXT(S_H_MOVE), NULL);
    x = hint(x, 0, TXT(S_H_SELECT), NULL);
    x = hint(x, 1, TXT(S_H_BACK), NULL);
    x = hint(x, 0, TXT(S_H_SCROLL), "L/R");
    return x;
}

/* barra de rolagem em pixels, na borda direita */
static double scroll_t = -10;
static void draw_scrollbar(float alpha) {
    float viewh = SCR_H - hint_h;
    if (page_h <= SCR_H + 4) return;
    float top = 112, bot = viewh - 8;
    if (cam > 80) top = 8;
    float track = bot - top, th = track * viewh / page_h;
    if (th < 24) th = 24;
    float max = page_h - SCR_H, k = max > 0 ? cam / max : 0;
    if (k > 1) k = 1;
    float y = top + (track - th) * k;
    int active = now_s - scroll_t < .6;
    vita2d_draw_rectangle(SCR_W - 10, top, 4, track, col_alpha(RGBA8(0x1A, 0x1A, 0x1A, 0xFF), alpha));
    vita2d_draw_rectangle(SCR_W - 10, (int)(y / 4) * 4, 4, th, col_alpha(active ? C_WHITE : C_BORDER, alpha));
}

static void draw_viewer(void) {
    vita2d_draw_rectangle(0, 0, SCR_W, SCR_H, RGBA8(0, 0, 0, 0xF2));
    if (viewer < 0 || viewer >= shot_count || !shot_tex[viewer]) return;
    float w = 896, h = 504;
    tex_draw(shot_tex[viewer], (SCR_W - w) / 2, 4, w, h, C_WHITE);
    char n[32];
    snprintf(n, sizeof(n), "%d/%d", viewer + 1, shot_count);
    title_draw(n, SCR_W - 24 - title_width(n, 2), SCR_H - 28, 2, C_WHITE);
}

/* ================================================================ navegação */

static void settings_save(void) {
    sceIoMkdir("ux0:data/WolffsStore", 0777);
    FILE *f = fopen("ux0:data/WolffsStore/settings.txt", "w");
    if (!f) return;
    fprintf(f, "lang=%s\nsound=%d\nmusic=%d\nost=%d\nbright=%d\n",
            lang == LANG_EN ? "en" : "pt", audio_enabled(), audio_music(), audio_ost_mode(), brightness);
    fclose(f);
}

static void settings_load(void) {
    FILE *f = fopen("ux0:data/WolffsStore/settings.txt", "r");
    if (!f) return;
    char l[64];
    while (fgets(l, sizeof(l), f)) {
        if (!strncmp(l, "lang=pt", 7)) lang = LANG_PT;
        if (!strncmp(l, "lang=en", 7)) lang = LANG_EN;
        if (!strncmp(l, "sound=0", 7)) audio_set_enabled(0);
        if (!strncmp(l, "music=0", 7)) audio_set_music(0);
        if (!strncmp(l, "ost=", 4)) {
            int mode = atoi(l + 4);
            if (mode >= 0 && mode <= 4) audio_set_ost_mode(mode);
        }
        if (!strncmp(l, "bright=", 7)) {
            brightness = atoi(l + 7);
            if (brightness < 40 || brightness > 100) brightness = 100;
        }
    }
    fclose(f);
}


static void cam_follow(void) {
    if (focus < 0 || focus >= nitems) return;
    Item *it = &items[focus];
    float top = it->y - 48, bot = it->y + it->h + 20;
    float viewh = SCR_H - hint_h;
    if (it->h + 96 > viewh) bot = top + viewh;
    if (top < cam_target) cam_target = top;
    if (bot > cam_target + viewh) cam_target = bot - viewh;
}

static void cam_clamp(void) {
    float max = page_h - SCR_H + (hint_h - 36);
    if (max < 0) max = 0;
    if (cam_target < 0) cam_target = 0;
    if (cam_target > max) cam_target = max;
}

static void set_focus(int f, int sound) {
    if (f == focus) return;
    focus = f;
    if (sound && f >= 0) sfx("hover");
    cam_follow();
    cam_clamp();
}

static void wipe(void) { wipe_t0 = now_s; }

static void go(int v, int arg) {
    int anim_home = v == V_HOME && view != V_HOME && started && now_s - intro_t0 >= T_DONE;
    if (anim_home) {
        home_t0 = now_s;
        sfx("home");
    }
    if ((v == V_DLS || v == V_SEARCH) && view != V_DLS && view != V_SEARCH) {
        dls_from = view;
        dls_from_arg = view == V_POST ? view_post : view_cat;
    }
    if (view != V_DLS && view != V_SEARCH) from_view = view;
    view = v;
    view_t0 = now_s;
    if (v == V_CAT) view_cat = arg;
    if (v == V_POST) {
        view_post = arg;
        load_shots(arg);
    } else {
        free_shots();
    }
    focus = -1;
    cam = cam_target = 0;
    rebuild();
    if (!anim_home) wipe(); /* no início a própria capa faz a transição */
    /* foco inicial */
    if (v == V_POST) focus = find_item(A_BACK, 0);
    else if (v == V_SEARCH) focus = find_item(A_SEARCHBOX, 0);
    else if (v == V_DLS) {
        focus = find_item(A_CANCEL, -1);
        for (int i = 0; i < nitems && focus < 0; i++)
            if (items[i].act == A_CANCEL) focus = i;
        if (focus < 0) focus = find_item(A_BACK, 0);
    } else if (v == V_CAT) {
        for (int i = 0; i < nitems; i++)
            if (items[i].act == A_POST) {
                focus = i;
                break;
            }
        if (focus < 0) focus = find_item(A_HOME, 0);
    } else {
        focus = find_item(A_POST, feat);
        /* o palco também é A_POST; prefira o botão "Ver detalhes" */
        for (int i = 0; i < nitems; i++)
            if (items[i].act == A_POST && items[i].arg == feat && items[i].h <= 96) focus = i;
    }
    if (v == V_HOME) cam = cam_target = 0; /* o início abre sempre no topo */
    else cam_follow();
    cam_clamp();
}

/* On Vita, the browser cannot render modern GitHub: use the static GitHub Vita
 * Lite mirror instead. The public website keeps its original github.com links.
 * Release asset downloads must retain their direct URL. */
static const char *vita_github_url(const char *url, char *out, size_t cap) {
    static const char *prefix = "https://github.com/";
    if (!url || strncmp(url, prefix, strlen(prefix)) != 0) return url;
    const char *path = url + strlen(prefix);
    if (!strncmp(path, "WolffsRoom", 10) &&
        (path[10] == 0 || path[10] == '?' || path[10] == '#')) {
        snprintf(out, cap, "https://wolffsroom.github.io/WolffsStore/");
        return out;
    }
    char owner[80] = "", repo[112] = "";
    if (sscanf(path, "%79[^/]/%111[^/?#]", owner, repo) != 2) return url;
    if (strstr(path, "/releases/download/")) return url; /* actual asset */
    const char *after = strchr(path, '/');
    if (!after) return url;
    after = strchr(after + 1, '/');
    const char *section = "";
    if (after && !strncmp(after, "/releases", 9)) section = "releases/";
    else if (after && !strncmp(after, "/issues", 7)) section = "issues/";
    snprintf(out, cap, "https://wolffsroom.github.io/WolffsStore/%s/%s/%s", owner, repo, section);
    return out;
}

static void open_url(const char *url) {
    if (!url) return;
    char mapped[520], uri[620];
    const char *target = vita_github_url(url, mapped, sizeof(mapped));
    snprintf(uri, sizeof(uri), "webmodal: %s", target);
    sceAppMgrLaunchAppByUri(0xFFFFF, uri);
}

/* ---------------------------------------------------------------- avisos (toasts) */

typedef struct { char text[192]; double t0; } Toast;
static Toast toasts[4];

static void toast(const char *msg) {
    memmove(&toasts[1], &toasts[0], sizeof(Toast) * 3);
    snprintf(toasts[0].text, sizeof(toasts[0].text), "%s", msg);
    toasts[0].t0 = now_s;
}

static void toggle_lang(void) {
    lang = !lang;
    settings_save();
    sfx("tick");
    int f = focus;
    rebuild();
    focus = f < nitems ? f : nitems - 1;
    cam_clamp();
}

static void toggle_sound(void) {
    audio_set_enabled(!audio_enabled());
    if (audio_enabled()) sfx("on");
    settings_save();
}

static void letters_wave(double at, int notes);

static void start_sync(int manual) {
    if (sync_state() == SYNC_RUNNING) return;
    sync_manual = manual;
    sync_reported = 0;
    sync_start();
    if (manual) {
        sfx("tick");
        letters_wave(now_s, 0);
        toast(TXT(S_SYNCING));
    }
}

/* ---------------------------------------------------------------- janela de configurações */

enum { SET_SFX, SET_MUSIC, SET_OST, SET_LANG, SET_BRIGHT, SET_COUNT };
enum { SETTINGS_ROOT, SETTINGS_AUDIO, SETTINGS_VISUAL, SETTINGS_GENERAL };

static int settings_rows(void) {
    return settings_page == SETTINGS_ROOT || settings_page == SETTINGS_AUDIO ? 3 : 1;
}
/* Visual and General have independent row lists so future settings can be
 * added without growing the root modal. */
static int settings_option(int row) {
    static const int audio_rows[] = {SET_SFX, SET_MUSIC, SET_OST};
    if (settings_page == SETTINGS_AUDIO && row >= 0 && row < 3)
        return audio_rows[row];
    if (settings_page == SETTINGS_VISUAL && row == 0) return SET_BRIGHT;
    if (settings_page == SETTINGS_GENERAL && row == 0) return SET_LANG;
    return -1;
}
static void settings_geometry(float *x, float *y, float *w, float *h, float *rh) {
    *w = 624;
    *rh = 58;
    *h = 148 + settings_rows() * *rh; /* 322 px max: always inside 960x544 */
    *x = (SCR_W - *w) / 2;
    *y = (SCR_H - *h) / 2;
}
static void open_settings(void) {
    settings_open = 1;
    settings_page = SETTINGS_ROOT;
    settings_row = settings_root_row = 0;
    settings_t0 = now_s;
    sfx("open");
}
static void close_settings(void) {
    settings_open = 0;
    settings_save();
    sfx("back");
}
static void settings_back(void) {
    if (settings_page != SETTINGS_ROOT) {
        settings_page = SETTINGS_ROOT;
        settings_row = settings_root_row;
        settings_t0 = now_s;
        settings_save();
        sfx("back");
    } else close_settings();
}
static void settings_change(int dir) {
    if (settings_page == SETTINGS_ROOT) {
        if (dir == 0) {
            settings_root_row = settings_row;
            settings_page = settings_row == 0 ? SETTINGS_AUDIO :
                            settings_row == 1 ? SETTINGS_VISUAL : SETTINGS_GENERAL;
            settings_row = 0;
            settings_t0 = now_s;
            sfx("open");
        }
        return;
    }
    switch (settings_option(settings_row)) {
    case SET_SFX:
        audio_set_enabled(!audio_enabled());
        if (audio_enabled()) sfx("on");
        break;
    case SET_MUSIC:
        audio_set_music(!audio_music());
        sfx("tick");
        break;
    case SET_OST: {
        int mode = audio_ost_mode();
        mode = (mode + (dir < 0 ? 4 : 1)) % 5;
        audio_ost_refresh_clock();
        audio_set_ost_mode(mode);
        sfx("tick");
        break;
    }
    case SET_LANG: toggle_lang(); break;
    case SET_BRIGHT:
        if (dir == 0) brightness = brightness >= 100 ? 40 : brightness + 10;
        else brightness += dir * 10;
        if (brightness > 100) brightness = 100;
        if (brightness < 40) brightness = 40;
        sfx("tick");
        break;
    }
    settings_save();
}
static void draw_settings(void) {
    if (!settings_open) return;
    double e = now_s - settings_t0;
    float k = e >= .16 ? 1 : ((int)(e / .04) + 1) / 4.f;
    vita2d_draw_rectangle(0, 0, SCR_W, SCR_H, RGBA8(0, 0, 0, (int)(0xD0 * k)));
    float x, y, w, h, rh;
    settings_geometry(&x, &y, &w, &h, &rh);
    y += 12 * (1 - k);
    halo_draw(x - 24, y - 24, w + 48, h + 48, 1, k);
    vita2d_draw_rectangle(x, y, w, h, col_alpha(C_PANEL, k));
    frame_rect(x, y, w, h, 4, col_alpha(C_WHITE, k));

    int category = settings_page == SETTINGS_AUDIO ? S_SET_CAT_AUDIO :
                   settings_page == SETTINGS_VISUAL ? S_SET_CAT_VISUAL : S_SET_CAT_GENERAL;
    const char *heading = settings_page == SETTINGS_ROOT ? TXT(S_SETTINGS) : TXT(category);
    char t[64];
    snprintf(t, sizeof(t), "%s", heading);
    upper_latin1(t);
    icon8(ICON_GEAR, x + 24, y + 26, 3, col_alpha(C_WHITE, k));
    title_draw_shadow(t, x + 68, y + 32, 2, col_alpha(C_WHITE, k), col_alpha(C_SHADOW, k));
    const char *ver = "Wolff's Store v" WS_VERSION;
    body_draw(ver, x + w - 24 - body_width(ver), y + 34, col_alpha(C_SOFT, k));
    vita2d_draw_rectangle(x + 24, y + 74, w - 48, 4, col_alpha(C_LINE, k));
    const int rows = settings_rows();
    static const int groups[3] = {S_SET_CAT_AUDIO, S_SET_CAT_VISUAL, S_SET_CAT_GENERAL};
    static const int options[SET_COUNT] = {S_SET_SFX, S_SET_MUSIC, S_SET_OST, S_SET_LANG, S_SET_BRIGHT};
    static const char *tracks[] = {"", "Original", "Morning Pixels", "Afternoon Arcade", "Night Shift"};

    for (int r = 0; r < rows; ++r) {
        float ry = y + 90 + r * rh, rx = x + 24, rw = w - 48;
        int focus_row = r == settings_row;
        if (focus_row) vita2d_draw_rectangle(rx, ry, rw, rh - 7, col_alpha(C_WHITE, k));
        unsigned tc = col_alpha(focus_row ? C_DARK : C_WHITE, k);
        int option = settings_option(r);
        const char *label = option < 0 ? TXT(groups[r]) : TXT(options[option]);
        body_draw_bold(label, rx + 18, ry + 15, tc);

        float vx = rx + rw - 19;
        /* Tiny pixel arrows stay consistent with the Store interface. */
        for (int i = 0; i < 4; ++i)
            vita2d_draw_rectangle(vx - 12 + i * 3, ry + 18 + i * 2, 3, 16 - i * 4, tc);
        if (settings_page == SETTINGS_ROOT) continue; /* submenu ▶, not switch */
        vx -= 22;
        if (option == SET_BRIGHT) {
            char pct[8];
            snprintf(pct, sizeof(pct), "%d%%", brightness);
            float pw = title_width(pct, 2);
            title_draw(pct, vx - pw, ry + 18, 2, tc);
            vx -= pw + 16;
            for (int step = 6; step >= 0; --step) {
                vx -= 12;
                vita2d_draw_rectangle(vx, ry + 18, 8, 16,
                    (40 + step * 10) <= brightness ? tc : col_alpha(focus_row ? C_SOFT : C_LINE, k));
            }
        } else {
            const char *value = option == SET_SFX ? TXT(audio_enabled() ? S_ON : S_OFF) :
                                option == SET_MUSIC ? TXT(audio_music() ? S_ON : S_OFF) :
                                option == SET_OST ? (audio_ost_mode() == 0 ? TXT(S_AUTO_OST) : tracks[audio_ost_mode()]) :
                                (lang == LANG_EN ? "English" : "Portugu\xeas (BR)");
            char up[48];
            snprintf(up, sizeof(up), "%s", value);
            upper_latin1(up);
            float vw = title_width(up, 2);
            title_draw(up, vx - vw, ry + 18, 2, tc);
            vx -= vw;
        }
        vx -= 26;
        for (int i = 0; i < 4; ++i)
            vita2d_draw_rectangle(vx + 9 - i * 3, ry + 18 + i * 2, 3, 16 - i * 4, tc);
    }
    float fy = y + 98 + rows * rh;
    vita2d_draw_rectangle(x + 24, fy, w - 48, 4, col_alpha(C_LINE, k));
    const char *hint = settings_page == SETTINGS_ROOT ?
                        (lang == LANG_PT ? "X abre submenu" : "X opens submenu") : TXT(S_SET_HINT);
    body_draw(hint, x + 32, fy + 18, col_alpha(C_SOFT, k));
    float cx = x + w - 126;
    btn_icon(1, cx, fy + 24);
    body_draw(TXT(S_H_BACK), cx + 22, fy + 18, col_alpha(C_SOFT, k));
}

/* ---------------------------------------------------------------- menu de áudio (como o do site) */

static const char *ICON_NOTE[8] = {"....###.", "...####.", "...#..#.", "...#..#.", "...#..#.", ".###.###", "####.###", ".##...#."};
static const char *ICON_SPARK[8] = {"...#....", "...#....", "..###...", "#######.", "..###...", "...#..#.", "...#.###", "......#."};

static void snd_menu_geom(float *x0, float *y0) {
    float cx = hdr_x[2] + hdr_w[2] / 2;
    *x0 = cx - (64 * 2 + 12) / 2;
    if (*x0 + 64 * 2 + 12 > 944) *x0 = 944 - (64 * 2 + 12);
    *y0 = 18 + BADGE + 26 + 10 - cam;
}

static void snd_menu_toggle(int row) {
    if (row == 0) {
        audio_set_music(!audio_music());
        sfx("tick");
    } else {
        audio_set_enabled(!audio_enabled());
        if (audio_enabled()) sfx("on");
    }
    settings_save();
}

static void draw_snd_menu(void) {
    if (!snd_menu) return;
    float x0, y0;
    snd_menu_geom(&x0, &y0);
    for (int r = 0; r < 2; r++) {
        float x = x0 + r * (64 + 12), y = y0;
        int on = r == 0 ? audio_music() : audio_enabled(), f = r == snd_row;
        vita2d_draw_rectangle(x + 4, y + 4, 64, 64, C_SH2);
        vita2d_draw_rectangle(x, y, 64, 64, f ? C_WHITE : C_DARK);
        frame_rect(x, y, 64, 64, 4, f ? C_WHITE : C_BTNBRD);
        unsigned ic = f ? C_DARK : (on ? C_WHITE : C_MUTED);
        icon8(r == 0 ? ICON_NOTE : ICON_SPARK, x + 16, y + 16, 4, ic);
        if (!on) /* desligado: traço na diagonal */
            for (int k = 0; k < 10; k++) vita2d_draw_rectangle(x + 10 + k * 4, y + 50 - k * 4, 4, 4, f ? C_DARK : C_WHITE);
    }
    const char *lab = TXT(snd_row == 0 ? S_MUSIC : S_SFX);
    float lw = body_width(lab), lx = x0 + (64 * 2 + 12) / 2 - lw / 2;
    vita2d_draw_rectangle(lx - 10, y0 + 64 + 8, lw + 20, 30, C_DARK);
    frame_rect(lx - 10, y0 + 64 + 8, lw + 20, 30, 4, C_BTNBRD);
    body_draw_bold(lab, lx, y0 + 64 + 13, C_WHITE);
}

/* toque com o menu aberto: liga/desliga o quadrado tocado; fora dele fecha. Devolve 1 se usou o toque. */
static int snd_menu_touch(float tx, float ty) {
    if (!snd_menu) return 0;
    float x0, y0;
    snd_menu_geom(&x0, &y0);
    for (int r = 0; r < 2; r++) {
        float x = x0 + r * (64 + 12);
        if (tx >= x && tx < x + 64 && ty >= y0 && ty < y0 + 64) {
            snd_row = r;
            snd_menu_toggle(r);
            return 1;
        }
    }
    if (tx >= hdr_x[2] && tx < hdr_x[2] + hdr_w[2] && ty + cam >= 18 && ty + cam < 18 + BADGE + 26) return 0; /* o próprio botão fecha */
    snd_menu = 0;
    sfx("back");
    return 1;
}

/* ---------------------------------------------------------------- vídeo em tela cheia */

static void close_video(void) {
    if (!vid_open) return;
    video_close();
    vid_open = 0;
    if (vid_music) audio_set_music(1);
    sfx("back");
}

static void draw_video(void) {
    if (!vid_open) return;
    vita2d_draw_rectangle(0, 0, SCR_W, SCR_H, RGBA8(0, 0, 0, 0xFF));
    video_draw();
    int st = video_state();
    if (st == VID_LOADING) {
        char t[64];
        snprintf(t, sizeof(t), "%s %d%%", TXT(S_VID_LOAD), (int)(video_load_progress() * 100));
        upper_latin1(t);
        title_draw(t, SCR_W / 2 - title_width(t, 2) / 2, SCR_H / 2 - 24, 2, C_WHITE);
        float w = 400, k = video_load_progress();
        frame_rect(SCR_W / 2 - w / 2, SCR_H / 2 + 8, w, 24, 4, C_WHITE);
        for (float sx = 0; sx < (w - 8) * k; sx += 12) vita2d_draw_rectangle(SCR_W / 2 - w / 2 + 4 + sx, SCR_H / 2 + 12, 8, 16, C_WHITE);
    } else if (st == VID_ERROR) {
        char *lines[4];
        static char buf[512];
        title_draw(TXT(S_VID_FAIL), 24, SCR_H / 2 - 48, 1, C_WHITE);
        int n = wrap_body(video_error(), 900, lines, 4, buf, sizeof(buf));
        for (int i = 0; i < n; i++) body_draw(lines[i], 24, SCR_H / 2 - 24 + i * BODY_LINE, C_SOFT);
    } else if (st == VID_PAUSED) {
        vita2d_draw_rectangle(SCR_W / 2 - 28, SCR_H / 2 - 32, 20, 64, C_WHITE);
        vita2d_draw_rectangle(SCR_W / 2 + 8, SCR_H / 2 - 32, 20, 64, C_WHITE);
    }
    /* dica e tempo no rodapé */
    vita2d_draw_rectangle(0, SCR_H - 36, SCR_W, 36, RGBA8(0x06, 0x06, 0x06, 0xC0));
    body_draw(TXT(S_VID_HINT), 24, SCR_H - 30, C_SOFT);
    unsigned long long ms = video_time_ms();
    char tm[16];
    snprintf(tm, sizeof(tm), "%02u:%02u", (unsigned)(ms / 60000), (unsigned)(ms / 1000 % 60));
    title_draw(tm, SCR_W - 24 - title_width(tm, 2), SCR_H - 26, 2, C_WHITE);
}

/* ---------------------------------------------------------------- teclado do Vita (pesquisa) */

static SceWChar16 ime_title[32], ime_init[128], ime_buf[128];

static void open_search(void) {
    if (ime_open) return;
    if (view != V_SEARCH) go(V_SEARCH, 0);
    const char *t = TXT(S_TIP_SEARCH);
    int i = 0;
    for (; t[i] && i < 31; i++) ime_title[i] = (unsigned char)t[i]; /* Latin-1 = primeiros 256 do Unicode */
    ime_title[i] = 0;
    for (i = 0; search_q[i] && i < 127; i++) ime_init[i] = (unsigned char)search_q[i];
    ime_init[i] = 0;
    memset(ime_buf, 0, sizeof(ime_buf));
    SceImeDialogParam p;
    sceImeDialogParamInit(&p);
    p.supportedLanguages = SCE_IME_LANGUAGE_ENGLISH | SCE_IME_LANGUAGE_PORTUGUESE;
    p.languagesForced = SCE_FALSE;
    p.type = SCE_IME_TYPE_DEFAULT;
    p.option = SCE_IME_OPTION_NO_AUTO_CAPITALIZATION;
    p.dialogMode = SCE_IME_DIALOG_DIALOG_MODE_WITH_CANCEL;
    p.textBoxMode = SCE_IME_DIALOG_TEXTBOX_MODE_WITH_CLEAR;
    p.title = ime_title;
    p.maxTextLength = 64;
    p.initialText = ime_init;
    p.inputTextBuffer = ime_buf;
    p.enterLabel = SCE_IME_ENTER_LABEL_SEARCH;
    if (sceImeDialogInit(&p) >= 0) {
        ime_open = 1;
        sfx("open");
    }
}

static void poll_ime(void) {
    if (!ime_open || sceImeDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) return;
    SceImeDialogResult r;
    memset(&r, 0, sizeof(r));
    sceImeDialogGetResult(&r);
    if (r.button == SCE_IME_DIALOG_BUTTON_ENTER) {
        int i = 0;
        for (; ime_buf[i] && i < (int)sizeof(search_q) - 1; i++) search_q[i] = ime_buf[i] < 256 ? (char)ime_buf[i] : '?';
        search_q[i] = 0;
        view_t0 = now_s;
        rebuild();
        focus = find_item(A_SEARCHBOX, 0);
        for (int k = 0; k < nitems; k++) /* foco no primeiro resultado */
            if (items[k].act == A_POST) {
                focus = k;
                break;
            }
        cam_follow();
        cam_clamp();
        sfx("peek");
    }
    sceImeDialogTerm();
    ime_open = 0;
}

static void start_intro(void);

static void activate(int it) {
    if (it < 0 || it >= nitems) return;
    Item *x = &items[it];
    pressed_item = it;
    pressed_t0 = now_s;
    switch (x->act) {
    case A_START: start_intro(); break;
    case A_SYNC: start_sync(1); break;
    case A_DLS:
        if (view == V_DLS) break;
        sfx("open");
        go(V_DLS, 0);
        break;
    case A_LANG: toggle_lang(); break;
    case A_SOUND:
        snd_menu = !snd_menu;
        snd_row = 0;
        sfx(snd_menu ? "open" : "back");
        break;
    case A_VIDEO:
        if (x->arg >= 0 && x->arg < g_nposts && g_posts[x->arg].video_file) {
            sfx("confirm");
            vid_music = audio_music();
            if (vid_music) audio_set_music(0);
            video_open(g_posts[x->arg].video_file, g_posts[x->arg].slug);
            vid_open = 1;
        }
        break;
    case A_SETTINGS: open_settings(); break;
    case A_SEARCH:
    case A_SEARCHBOX: open_search(); break;
    case A_REGUPD: {
        RegEntry *e = reg_get(x->arg);
        if (!e) break;
        for (int i = 0; i < g_nposts; i++)
            if (!strcmp(g_posts[i].slug, e->slug) && e->dl < g_posts[i].ndls) {
                if (jobs_add(&g_posts[i], e->dl, e->name[0] ? e->name : g_posts[i].dls[e->dl].name) >= 0) sfx("confirm");
                break;
            }
        break;
    }
    case A_LAUNCH: { /* abre o jogo/app instalado */
        char tid[16] = "";
        if (x->url) snprintf(tid, sizeof(tid), "%s", x->url);
        else if (view == V_POST && x->arg >= 0 && x->arg < g_posts[view_post].ndls) {
            DlStatus st;
            dl_status(&g_posts[view_post], x->arg, &st);
            snprintf(tid, sizeof(tid), "%s", st.tid);
        }
        if (tid[0]) {
            char uri[48];
            snprintf(uri, sizeof(uri), "psgm:play?titleid=%s", tid);
            sfx("confirm");
            int r = sceAppMgrLaunchAppByUri(0xFFFFF, uri);
            if (r < 0) ws_log("abrir %s falhou: 0x%08X", tid, r);
        }
        break;
    }
    case A_DL: {
        if (view != V_POST || view_post < 0) break;
        Post *p = &g_posts[view_post];
        Job *j = jobs_find(p->slug, x->arg);
        if (j && j->state < J_DONE) { /* já está baixando: mostra a fila */
            sfx("open");
            go(V_DLS, 0);
            break;
        }
        char title[160];
        snprintf(title, sizeof(title), "%s", p->dls[x->arg].name);
        if (jobs_add(p, x->arg, title) >= 0) sfx("confirm");
        break;
    }
    case A_CANCEL:
        jobs_cancel(x->arg);
        sfx("back");
        break;
    case A_CLEAR:
        jobs_clear_finished();
        sfx("tick");
        rebuild();
        focus = find_item(A_BACK, 0);
        break;
    case A_POST:
        sfx("open");
        go(V_POST, x->arg);
        break;
    case A_PEEK:
        if (peek == x->arg) {
            peek = -1;
            sfx("back");
            rebuild();
        } else {
            peek = x->arg;
            peek_t0 = now_s;
            sfx("peek");
            rebuild();
            /* desce até os cards */
            int f = -1;
            for (int i = 0; i < nitems; i++)
                if (items[i].act == A_POST && items[i].y > items[it].y) {
                    f = i;
                    break;
                }
            if (f < 0) f = find_item(A_ALL, peek);
            if (f >= 0) {
                focus = f;
                cam_follow();
                cam_clamp();
            }
        }
        break;
    case A_ALL:
        sfx("open");
        go(V_CAT, x->arg);
        break;
    case A_HOME:
        go(V_HOME, 0);
        break;
    case A_BACK: {
        if (view == V_DLS || view == V_SEARCH) {
            sfx("back");
            if (dls_from == V_POST && dls_from_arg >= 0 && dls_from_arg < g_nposts) go(V_POST, dls_from_arg);
            else if (dls_from == V_CAT) go(V_CAT, dls_from_arg);
            else go(V_HOME, 0);
            break;
        }
        int pi = view_post;
        if (from_view == V_CAT) {
            sfx("back");
            go(V_CAT, view_cat);
        } else {
            go(V_HOME, 0);
        }
        /* categoria: volta com o foco no card de onde saiu; início: sempre no topo */
        if (view == V_CAT) {
            for (int i = 0; i < nitems; i++)
                if (items[i].act == A_POST && items[i].arg == pi && items[i].h > 96) {
                    focus = i;
                    break;
                }
            cam_follow();
            cam_clamp();
        } else {
            cam = cam_target = 0;
        }
        break;
    }
    case A_URL:
        sfx("confirm");
        open_url(x->url);
        break;
    case A_SHOT:
        sfx("open");
        viewer = x->arg;
        break;
    case A_TOP:
        if (view != V_HOME) go(V_HOME, 0);
        else sfx("up");
        cam_target = 0;
        break;
    }
}

static void back(void) {
    if (viewer >= 0) {
        viewer = -1;
        sfx("back");
        return;
    }
    if (view == V_POST || view == V_DLS || view == V_SEARCH) {
        int it = find_item(A_BACK, 0);
        activate(it);
    } else if (view == V_CAT) {
        go(V_HOME, 0); /* volta ao início sempre no topo */
    } else if (peek >= 0) {
        int c = peek;
        peek = -1;
        sfx("back");
        rebuild();
        focus = find_item(A_PEEK, c);
        cam_target = 0;
    }
}

static int visible(const Item *it) { return it->y + it->h > cam && it->y < cam + SCR_H - hint_h; }

/* dir: 0 cima, 1 baixo, 2 esquerda, 3 direita */
static void move(int dir) {
    if (focus < 0 || !visible(&items[focus])) {
        /* o foco saiu da tela (rolagem manual): pega o item visível mais próximo do centro */
        int best = -1;
        float bd = 1e9f;
        for (int i = 0; i < nitems; i++) {
            if (!visible(&items[i])) continue;
            float cx = items[i].x + items[i].w / 2 - SCR_W / 2, cy = items[i].y + items[i].h / 2 - (cam + SCR_H / 2);
            float d = cx * cx + cy * cy;
            if (d < bd) {
                bd = d;
                best = i;
            }
        }
        if (best >= 0) {
            set_focus(best, 1);
            return;
        }
    }
    int best = -1;
    float bs = 1e9f;
    if (focus >= 0) {
        Item *a = &items[focus];
        float ax = a->x + a->w / 2, ay = a->y + a->h / 2;
        for (int i = 0; i < nitems; i++) {
            if (i == focus) continue;
            Item *b = &items[i];
            float bx = b->x + b->w / 2, by = b->y + b->h / 2;
            float dx = bx - ax, dy = by - ay, s;
            if (dir == 0) {
                if (b->y + b->h > a->y + 2) continue;
                s = -dy + fabsf(dx) * 2;
            } else if (dir == 1) {
                if (b->y < a->y + a->h - 2) continue;
                s = dy + fabsf(dx) * 2;
            } else if (dir == 2) {
                if (b->x + b->w > a->x + 2 || b->y > a->y + a->h || b->y + b->h < a->y) continue;
                s = -dx + fabsf(dy) * 3;
            } else {
                if (b->x < a->x + a->w - 2 || b->y > a->y + a->h || b->y + b->h < a->y) continue;
                s = dx + fabsf(dy) * 3;
            }
            /* não pula itens ainda escondidos (intro) */
            if (b->pop > now_s) continue;
            if (s < bs) {
                bs = s;
                best = i;
            }
        }
    }
    if (best >= 0) {
        /* para baixo/cima com texto longo no meio: rola antes de pular */
        Item *b = &items[best];
        float viewh = SCR_H - hint_h;
        if (dir == 1 && b->y > cam_target + viewh + 120) {
            cam_target += 160;
            cam_clamp();
            sfx("tick");
            return;
        }
        if (dir == 0 && b->y + b->h < cam_target - 120) {
            cam_target -= 160;
            cam_clamp();
            sfx("tick");
            return;
        }
        set_focus(best, 1);
    } else if (dir <= 1) {
        float old = cam_target;
        cam_target += dir ? 160 : -160;
        cam_clamp();
        if (cam_target != old) sfx("tick");
    }
}

/* ================================================================ intro */

typedef struct { double t; int kind, arg; } Cue; /* t: tempo absoluto (now_s) */
static Cue cues[128];
static int ncues, cue_next;

static int cue_cmp(const void *a, const void *b) {
    double d = ((const Cue *)a)->t - ((const Cue *)b)->t;
    return d < 0 ? -1 : d > 0;
}

static void cue(double t, int kind, int arg) {
    /* descarta os já tocados para abrir espaço */
    if (cue_next > 0) {
        memmove(cues, cues + cue_next, (ncues - cue_next) * sizeof(Cue));
        ncues -= cue_next;
        cue_next = 0;
    }
    if (ncues >= 128) return;
    cues[ncues++] = (Cue){t, kind, arg};
    qsort(cues, ncues, sizeof(Cue), cue_cmp);
}

/* Ao abrir o app: boot e a digitação da pergunta */
static void title_cues(void) {
    ncues = cue_next = 0;
    cue(title_t0 + .12, 0, 0);
    char *lines[6];
    static char buf[512];
    int n = kicker_layout(lines, buf, sizeof(buf)), idx = 0;
    for (int l = 0; l < n; l++) {
        for (int i = 0; lines[l][i]; i++, idx++)
            if (lines[l][i] != ' ') cue(title_t0 + kick_char_t(idx), 1, 0);
        idx++;
    }
}

/* Primeira execução: sincroniza antes da pergunta da intro. A pata e o logo
 * continuam vivos enquanto o catálogo e as imagens são preparados. */
static void draw_first_sync(void) {
    const int sc = 5, step = 8 * sc;
    paw_draw(SCR_W / 2 - 36, SCR_H / 2 - 132, 1, 1);
    float lw = title_width(LOGO, sc);
    for (int i = 0; LOGO[i]; i++) {
        char ch[2] = {LOGO[i], 0};
        double e = now_s - letter_hop[i];
        float hop = e >= 0 && e < .16 ? -10 : 0;
        float x = (SCR_W - lw) / 2 + i * step;
        title_draw(ch, x + 5, SCR_H / 2 - 38 + 5 + hop, sc, C_SHADOW);
        title_draw(ch, x, SCR_H / 2 - 38 + hop, sc, C_WHITE);
    }
    const char *msg = TXT(S_SYNCING);
    body_draw(msg, (SCR_W - body_width(msg)) / 2, SCR_H / 2 + 34, C_SOFT);
    char pc[16];
    snprintf(pc, sizeof(pc), "%d%%", (int)(sync_progress() * 100));
    title_draw(pc, (SCR_W - title_width(pc, 2)) / 2, SCR_H / 2 + 66, 2, C_WHITE);
}

static void letters_wave(double at, int notes) {
    wave_t0 = at;
    if (!notes) return;
    for (int i = 0; LOGO[i]; i++)
        if (LOGO[i] != ' ') cue(at + i * .06, 6, i);
}

static void start_intro(void) {
    if (started) return;
    started = 1;
    intro_t0 = now_s;
    ncues = cue_next = 0; /* para a digitação, se ainda estiver rolando */
    cue(now_s, 2, 0);                                   /* TV desligando */
    cue(now_s + T_TV, 3, 0);                            /* TV ligando */
    for (int m = 0; m < 8; m++) cue(now_s + T_MOS + m * .13, 4, m);
    cue(now_s + T_MOVE, 7, 0);                          /* capa encaixa */
    letters_wave(now_s + T_HEAD + .1, 1);               /* letras do logo */
    cue(now_s + T_NAV + .3, 5, 0);                      /* ready */
    rebuild();
    focus = -1;
}

static void run_cues(void) {
    while (cue_next < ncues && cues[cue_next].t <= now_s) {
        Cue *c = &cues[cue_next++];
        switch (c->kind) {
        case 0: sfx("boot"); break;
        case 1: if (!started) sfx("type"); break;
        case 2: sfx("off"); break;
        case 3: sfx("tvon"); break;
        case 4: sfx_mos(c->arg); break;
        case 5:
            sfx("ready");
            if (view == V_HOME && focus < 0) {
                for (int i = 0; i < nitems; i++)
                    if (items[i].act == A_POST && items[i].arg == feat && items[i].h <= 96) focus = i;
            }
            break;
        case 6: sfx_letter(c->arg); break;
        case 7: sfx("up"); break;
        }
    }
}

/* ================================================================ recursos */

static void free_mosaic(void) {
    for (int m = 0; m < 8; m++)
        if (mos_tex[m]) {
            vita2d_free_texture(mos_tex[m]);
            mos_tex[m] = NULL;
        }
}

static void make_mosaic(void) {
    /* as 8 camadas da depixelização (larguras 12..288), por média de blocos, lidas da própria capa */
    vita2d_texture *c = cover_tex[feat];
    free_mosaic();
    if (!c) return;
    int w = vita2d_texture_get_width(c), h = vita2d_texture_get_height(c);
    int stride = vita2d_texture_get_stride(c);
    int bpp = vita2d_texture_get_format(c) == SCE_GXM_TEXTURE_FORMAT_U8U8U8_BGR ? 3 : 4; /* JPG / PNG */
    const unsigned char *px = vita2d_texture_get_datap(c);
    for (int m = 0; m < 8; m++) {
        int mw = MOS_W[m], mh = (mw * 9 + 8) / 16;
        unsigned char *out = malloc((size_t)mw * mh * 4);
        if (!out) break;
        for (int y = 0; y < mh; y++)
            for (int x = 0; x < mw; x++) {
                int x0 = x * w / mw, x1 = (x + 1) * w / mw, y0 = y * h / mh, y1 = (y + 1) * h / mh;
                unsigned s[4] = {0}, n = 0;
                for (int yy = y0; yy < y1; yy++)
                    for (int xx = x0; xx < x1; xx++, n++) {
                        const unsigned char *q = px + (size_t)yy * stride + (size_t)xx * bpp;
                        s[0] += q[0];
                        s[1] += q[1];
                        s[2] += q[2];
                        s[3] += bpp == 4 ? q[3] : 255;
                    }
                for (int k = 0; k < 4; k++) out[((size_t)y * mw + x) * 4 + k] = n ? s[k] / n : 0;
            }
        mos_tex[m] = tex_from_rgba(out, mw, mh, 1);
        free(out);
    }
}

static void free_covers(void) {
    for (int i = 0; i < MAX_POSTS; i++)
        if (cover_tex[i]) {
            vita2d_free_texture(cover_tex[i]);
            cover_tex[i] = NULL;
        }
}

static void load_covers(void) {
    for (int i = 0; i < g_nposts; i++)
        if (g_posts[i].cover) cover_tex[i] = load_image(g_posts[i].cover_url, g_posts[i].slug, "cover.wrt", g_posts[i].pixel);
}

/* ================================================================ loja: sync, fila e auto-atualização */

/* Verify the installed helper artwork byte-for-byte, not only filenames. */
static int updater_asset_matches(const char *relative) {
    char src[240], installed[240];
    snprintf(src, sizeof(src), WS_PKG_SELF "/updater_v2/sce_sys/%s", relative);
    snprintf(installed, sizeof(installed), "ux0:app/WFFS00043/sce_sys/%s", relative);
    int a_len = 0, b_len = 0;
    char *a = fs_read(src, &a_len);
    char *b = fs_read(installed, &b_len);
    int match = a && b && a_len == b_len && memcmp(a, b, (size_t)a_len) == 0;
    ws_log("updater LiveArea %s: %s (%d/%d bytes)", relative,
           match ? "verified" : "MISMATCH", a_len, b_len);
    free(a);
    free(b);
    return match;
}

/* The self-update worker extracted the NEXT release into WS_PKG_SELF.
 * The new app installs updater_v2/ as WFFS00043. updater/ is a legacy
 * compatibility bridge for the old WOLF00001 / WOLF00002 updater code. */
static int copy_new_updater_file(const char *relative, const char *dst) {
    char src[512];
    snprintf(src, sizeof(src), WS_PKG_SELF "/updater_v2/%s", relative);
    int r = fs_copy(src, dst);
    if (r < 0) ws_log("updater: NEW VPK missing %s (0x%08X)", src, r);
    return r == 0;
}

static void launch_updater(void) {
    /* instala o updater (WFFS00043) a partir do que veio no VPK e passa a vez para ele */
    fs_rm_rf(WS_PKG_UPD);
    fs_mkdir_p(WS_PKG_UPD "/sce_sys");
    fs_mkdir_p(WS_PKG_UPD "/sce_sys/livearea/contents");
    fs_mkdir_p(WS_PKG_UPD "/data/tex");
    /* All files, including icon and LiveArea, MUST come from NEW release. */
    int ok = copy_new_updater_file("eboot.bin", WS_PKG_UPD "/eboot.bin") &&
             copy_new_updater_file("sce_sys/param.sfo", WS_PKG_UPD "/sce_sys/param.sfo") &&
             copy_new_updater_file("data/tex/paw4.wrt", WS_PKG_UPD "/data/tex/paw4.wrt") &&
             copy_new_updater_file("data/tex/font_title.wrt", WS_PKG_UPD "/data/tex/font_title.wrt") &&
             copy_new_updater_file("sce_sys/icon0.png", WS_PKG_UPD "/sce_sys/icon0.png") &&
             copy_new_updater_file("sce_sys/livearea/contents/bg0.png", WS_PKG_UPD "/sce_sys/livearea/contents/bg0.png") &&
             copy_new_updater_file("sce_sys/livearea/contents/startup.png", WS_PKG_UPD "/sce_sys/livearea/contents/startup.png") &&
             copy_new_updater_file("sce_sys/livearea/contents/template.xml", WS_PKG_UPD "/sce_sys/livearea/contents/template.xml");
    if (ok) ok = inst_make_head_bin(WS_PKG_UPD) == 0;
    /* Delete the OLD helper while Store is foreground, BEFORE the new helper
     * starts. This refreshes icon0 and the LiveArea on every upgrade. */
    if (ok && fs_exists("ux0:app/WFFS00043/sce_sys/param.sfo")) {
        int remove_old = inst_delete("WFFS00043");
        ws_log("updater: remove previous LiveArea/bubble -> 0x%08X", remove_old);
        if (remove_old < 0) {
            toast("Nao foi possivel substituir o Auto Updater antigo.");
            upd_job = -1;
            upd_skipped = 1;
            return;
        }
        sceKernelDelayThread(350 * 1000); /* let SceShell invalidate old bubble */
    }
    if (ok) ok = inst_promote(WS_PKG_UPD) == 0;
    if (ok) {
        /* Refuse handoff to a helper whose LiveArea was not installed. */
        ok = fs_exists("ux0:app/WFFS00043/sce_sys/icon0.png") &&
             fs_exists("ux0:app/WFFS00043/sce_sys/livearea/contents/bg0.png") &&
             fs_exists("ux0:app/WFFS00043/sce_sys/livearea/contents/startup.png") &&
             fs_exists("ux0:app/WFFS00043/sce_sys/livearea/contents/template.xml");
        if (ok) {
            ok = updater_asset_matches("icon0.png") &&
                 updater_asset_matches("livearea/contents/bg0.png") &&
                 updater_asset_matches("livearea/contents/startup.png") &&
                 updater_asset_matches("livearea/contents/template.xml");
        }
        if (!ok) ws_log("updater: installed bubble missing custom LiveArea files or stale asset bytes");
    }
    if (!ok) {
        ws_log("updater: required files from downloaded release could not be installed/validated");
        toast("Nao foi possivel instalar o updater da nova versao.");
        upd_job = -1;
        upd_skipped = 1; /* no repeated destructive install attempts */
        return;
    }
    /* VitaShell-style shell handoff. 0x20000 requested the second game while
     * Store was still foreground and made the user confirm closing Store.
     * Queue a shell launch once, then exit immediately. No second launch:
     * repeated requests provoke another "close the updater?" dialog. */
    vita2d_wait_rendering_done();
    ws_log("updater: VitaShell-style handoff to WFFS00043");
    int launched = sceAppMgrLaunchAppByUri(0x20000, "psgm:play?titleid=WFFS00043");
    ws_log("updater: direct app launch returned 0x%08X", launched);
    if (launched >= 0) sceKernelExitProcess(0);
    toast("Falha ao abrir o Auto Updater. Abra WFFS00043 pela LiveArea.");
}

static void maybe_self_update(void) {
    if (upd_job >= 0 || upd_skipped || !app_rel.url[0] || !app_rel.version[0]) return;
    if (version_cmp(WS_VERSION, app_rel.version) >= 0) return;
    upd_job = jobs_add_self(&app_rel);
}

static void apply_sync(void) {
    static Post fresh[MAX_POSTS];
    AppRelease app;
    int n = sync_take(fresh, MAX_POSTS, &app);
    if (n <= 0) return;
    char cur[48] = "";
    if (view_post >= 0 && view_post < g_nposts) snprintf(cur, sizeof(cur), "%s", g_posts[view_post].slug);
    vita2d_wait_rendering_done();
    free_covers();
    free_shots();
    memcpy(g_posts, fresh, sizeof(Post) * n);
    g_nposts = n;
    app_rel = app;
    feat = catalog_featured();
    load_covers();
    make_mosaic();
    view_post = -1;
    for (int i = 0; i < g_nposts && cur[0]; i++)
        if (!strcmp(g_posts[i].slug, cur)) view_post = i;
    if (view == V_POST && view_post < 0) {
        go(V_HOME, 0);
    } else {
        if (view == V_POST) load_shots(view_post);
        int f = focus;
        rebuild();
        focus = f < nitems ? f : nitems - 1;
        cam_clamp();
    }
    if (sync_manual) toast(TXT(S_SYNC_OK));
}

static unsigned jobs_seen_version;

/* Deleting a registered app may briefly refresh SceShell. Keep the first
 * Store session usable by delaying cleanup until Home is fully stable.
 * Verify both TITLE_ID and TITLE from installed param.sfo; never delete a
 * different homebrew occupying a previously used helper ID. */
static SceUID updater_cleanup_thread = -1;
static int updater_cleanup_owned(const char *id) {
    char sfo[140], installed_id[32] = "", installed_title[100] = "";
    snprintf(sfo, sizeof(sfo), "ux0:app/%s/sce_sys/param.sfo", id);
    return fs_exists(sfo) &&
           sfo_get(sfo, "TITLE_ID", installed_id, sizeof(installed_id)) == 0 &&
           strcmp(installed_id, id) == 0 &&
           sfo_get(sfo, "TITLE", installed_title, sizeof(installed_title)) == 0 &&
           strcmp(installed_title, "Wolff's Store Updater") == 0;
}
static int updater_cleanup_run(SceSize arglen, void *args) {
    (void)arglen; (void)args;
    int pending=0;
    const char *ids[]={"WFFS00043", "WOLF00002"};
    const int requested[]={updater_cleanup_pending, legacy_updater_cleanup_pending};
    for (int i=0;i<2;i++) {
        if (!requested[i]) continue;
        char sfo[140];
        snprintf(sfo, sizeof(sfo),"ux0:app/%s/sce_sys/param.sfo",ids[i]);
        if (!fs_exists(sfo)) continue;
        if (!updater_cleanup_owned(ids[i])) {
            ws_log("updater: refusing cleanup of unverified title %s",ids[i]);
            pending=1;
            continue;
        }
        ws_log("updater: Home stable, removing verified helper %s",ids[i]);
        int r=inst_delete(ids[i]);
        ws_log("updater: delayed DeletePkg(%s) -> 0x%08X",ids[i],r);
        if (r < 0 || fs_exists(sfo)) pending=1;
    }
    if (!pending) {
        fs_rm_rf(WS_DATA "/updater_cleanup.pending");
        updater_cleanup_pending=legacy_updater_cleanup_pending=0;
        ws_log("updater: delayed cleanup completed, marker removed");
    } else ws_log("updater: some helpers remain; marker retained for later retry");
    return sceKernelExitDeleteThread(0);
}
static void start_updater_cleanup_once(void) {
    if (updater_cleanup_thread >= 0) return;
    updater_cleanup_thread = sceKernelCreateThread(
        "ws_updater_cleanup", updater_cleanup_run, 0x10000100, 0x8000, 0, 0, NULL);
    if (updater_cleanup_thread < 0) {
        ws_log("updater: unable to create cleanup worker 0x%08X",updater_cleanup_thread);
        return;
    }
    int r=sceKernelStartThread(updater_cleanup_thread,0,NULL);
    if (r < 0) {
        ws_log("updater: unable to start cleanup worker 0x%08X",r);
        sceKernelDeleteThread(updater_cleanup_thread);
        updater_cleanup_thread=-1;
    } else ws_log("updater: deferred cleanup started after stable Home");
}

static void tick_store(void) {
    int intro_done = started && now_s - intro_t0 >= T_DONE;
    /* Remove only our updater, only after the Home has been continuously
     * stable. This avoids SceShell refresh during the fragile first frames
     * after the updater -> new Store handoff. The deletion happens once,
     * on a worker so it cannot freeze the main input/render loop. */
    static double cleanup_stable_since = -1;
    static int cleanup_attempted_this_session = 0;
    int stable_home = !first_sync && started && intro_done &&
        view == V_HOME && viewer < 0 && !settings_open &&
        !vid_open && !ime_open && upd_job < 0 && jobs_active() == 0;
    if (updater_cleanup_pending || legacy_updater_cleanup_pending) {
        if (stable_home) {
            if (cleanup_stable_since < 0) {
                cleanup_stable_since = now_s;
                ws_log("updater: Home stable, starting guarded 45-second grace period");
            }
            if (!cleanup_attempted_this_session &&
                now_s - cleanup_stable_since >= 45.0) {
                cleanup_attempted_this_session = 1;
                start_updater_cleanup_once();
            }
        } else if (!cleanup_attempted_this_session) cleanup_stable_since = -1;
    }
    int st = sync_state();
    if (first_sync && st == SYNC_READY) {
        apply_sync();
        first_sync = 0;
        title_t0 = now_s;
        title_cues();
        return;
    }
    if (first_sync && (st == SYNC_OFFLINE || st == SYNC_FAIL)) {
        first_sync = 0;
        title_t0 = now_s;
        title_cues();
        return;
    }
    /* resultado da sincronização: só troca o catálogo com a intro terminada */
    if (st == SYNC_READY && intro_done && viewer < 0) apply_sync();
    if ((st == SYNC_OFFLINE || st == SYNC_FAIL) && !sync_reported && intro_done) {
        sync_reported = 1;
        toast(TXT(st == SYNC_OFFLINE ? S_SYNC_OFF : S_SYNC_FAIL));
    }
    if (intro_done) maybe_self_update();
    /* trabalhos que terminaram */
    if (jobs_version() != jobs_seen_version) {
        jobs_seen_version = jobs_version();
        int touched = 0;
        for (int i = 0; i < jobs_count(); i++) {
            Job *j = jobs_get(i);
            if (!j || j->seen || j->state < J_DONE) continue;
            j->seen = 1;
            touched = 1;
            if (j->kind == JK_SELF) {
                if (j->state == J_DONE) launch_updater();
                else {
                    char t[256];
                    snprintf(t, sizeof(t), "%s %s: %s", j->name, TXT(S_T_FAILED), j->msg);
                    if (j->state != J_CANCELED) toast(t);
                    upd_job = -1;
                    upd_skipped = 1;
                }
                continue;
            }
            char t[256];
            if (j->state == J_DONE) {
                /* guarda o que foi instalado e em qual versão */
                int kind = j->kind == JK_TRANS ? DL_TRANS : j->kind == JK_PLUGIN ? DL_PLUGIN : j->kind == JK_FILE ? DL_FILE
                         : j->kind == JK_DATA ? DL_DATA : DL_VPK;
                char path[192];
                if (kind == DL_TRANS) snprintf(path, sizeof(path), "ux0:rePatch/%s", j->tids[0]);
                else snprintf(path, sizeof(path), "%s", j->msg);
                reg_set(j->slug, j->dl, kind, j->version, j->name, path);
                sfx("ready");
                if (j->kind == JK_VPK) snprintf(t, sizeof(t), "%s %s", j->name, TXT(S_T_INSTALLED));
                else if (j->kind == JK_TRANS) snprintf(t, sizeof(t), "%s: %s (%s)", j->name, TXT(S_T_TRANS), j->msg);
                else if (j->kind == JK_DATA) snprintf(t, sizeof(t), "%s: %s", j->name, TXT(S_DATA_TO));
                else snprintf(t, sizeof(t), "%s %s", j->name, TXT(S_T_SAVED));
            } else if (j->state == J_ERROR) {
                sfx("back");
                snprintf(t, sizeof(t), "%s %s: %s", j->name, TXT(S_T_FAILED), j->msg);
            } else {
                snprintf(t, sizeof(t), "%s: %s", j->name, TXT(S_CANCELED));
            }
            toast(t);
        }
        /* telas que mostram estado de downloads se refazem */
        if (view == V_DLS || (touched && view == V_POST)) {
            int f = focus;
            rebuild();
            focus = f < nitems ? f : nitems - 1;
            if (view == V_DLS && (focus < 0 || items[focus].act == A_NONE)) focus = find_item(A_BACK, 0);
            cam_clamp();
        }
    }
}

/* Faixa de status logo acima da barra de dicas (como as notificações do PKGj):
 * mostra o download atual ou o último aviso. O conteúdo da página nunca fica por baixo dela,
 * porque a área útil da tela diminui enquanto ela aparece. */
static Job *current_job(void) {
    Job *j = NULL;
    for (int i = 0; i < jobs_count(); i++) {
        Job *k = jobs_get(i);
        if (k && k->state > J_QUEUED && k->state < J_DONE && k->kind != JK_SELF) j = k;
    }
    return j;
}

static int strip_visible(void) {
    if (upd_job >= 0 || !started || now_s - intro_t0 < T_DONE) return 0;
    if (toasts[0].text[0] && now_s - toasts[0].t0 < 4.0) return 1;
    return current_job() != NULL;
}

/* Status/toasts live INSIDE the existing shortcut bar, not in an extra row.
 * Long messages cycle through visible characters as a pixel-text marquee. */
static void draw_strip(float hint_right) {
    if (!strip_visible()) return;
    float x = hint_right + 6, avail = SCR_W - x - 16;
    if (avail < 100) return; /* shortcuts always take priority */
    char msg[512] = "";
    Toast *t = &toasts[0];
    double e = now_s - t->t0;
    if (t->text[0] && e < 4.0) {
        snprintf(msg, sizeof(msg), "! %s", t->text);
    } else {
        Job *j = current_job();
        if (!j) return;
        float k = job_frac(j);
        if (j->state == J_DOWNLOAD) {
            char size[24] = "";
            fmt_size((long long)j->pr.speed, size, sizeof(size));
            snprintf(msg, sizeof(msg), "%s - %s %d%% %s/s",
                     j->name, job_state_text(j), (int)(k * 100), size);
        } else {
            snprintf(msg, sizeof(msg), "%s - %s", j->name, job_state_text(j));
        }
        int n = jobs_active();
        if (n > 1) {
            size_t off = strlen(msg);
            snprintf(msg + off, sizeof(msg) - off, " (+%d)", n - 1);
        }
    }
    float y = SCR_H - 30 + hint_slide;
    vita2d_draw_rectangle(x - 4, SCR_H - 32 + hint_slide, SCR_W - x - 12, 30,
                          RGBA8(0x06, 0x06, 0x06, 0xFF));
    if (body_width(msg) <= avail) {
        body_draw(msg, x + avail - body_width(msg), y, C_SOFT);
        return;
    }
    /* Character-stepped loop avoids drawing outside the free space (no clip API). */
    static char cycle[600];
    snprintf(cycle, sizeof(cycle), "%s     *     ", msg);
    int len = (int)strlen(cycle);
    if (!len) return;
    int offset = ((int)(now_s * 9.0)) % len;
    char visible[256] = "";
    for (int i = 0; i < len && i < (int)sizeof(visible) - 1; i++) {
        int n = (int)strlen(visible);
        visible[n] = cycle[(offset + i) % len];
        visible[n + 1] = 0;
        if (body_width(visible) > avail) { visible[n] = 0; break; }
    }
    body_draw(visible, x, y, C_SOFT);
}

/* tela da atualização do próprio app */
static void draw_update(void) {
    Job *j = upd_job >= 0 ? job_by_id(upd_job) : NULL;
    if (!j || j->state >= J_DONE) return;
    vita2d_draw_rectangle(0, 0, SCR_W, SCR_H, RGBA8(0, 0, 0, 0xE6));
    float w = 680, h = 330, x = (SCR_W - w) / 2, y = (SCR_H - h) / 2;
    halo_draw(x - 28, y - 28, w + 56, h + 56, 1, 1);
    vita2d_draw_rectangle(x, y, w, h, C_PANEL);
    frame_rect(x, y, w, h, 4, C_WHITE);
    const char *brand = "WOLFF'S ROOM";
    float brand_x = x + (w - title_width(brand, 3)) / 2;
    for (int i = 0; brand[i]; i++) {
        char ch[2] = {brand[i], 0};
        float bounce = -fabsf(sinf((float)now_s * 4.8f - i * .48f)) * 7.f;
        float cx = brand_x + i * 24;
        title_draw(ch, cx + 4, y + 48 + 4 + bounce, 3, C_SHADOW);
        title_draw(ch, cx, y + 48 + bounce, 3, C_WHITE);
    }
    char t[96];
    snprintf(t, sizeof(t), "%s", TXT(S_UPD_TITLE));
    upper_latin1(t);
    title_draw_shadow(t, x + (w - title_width(t, 2)) / 2, y + 96, 2, C_WHITE, C_SHADOW);
    char v[64];
    snprintf(v, sizeof(v), "v%s > v%s", WS_VERSION, app_rel.version);
    title_draw(v, x + (w - title_width(v, 2)) / 2, y + 128, 2, C_SOFT);
    float k = job_frac(j);
    char s[96];
    if (j->state == J_DOWNLOAD) snprintf(s, sizeof(s), "%s %d%%", job_state_text(j), (int)(k * 100));
    else snprintf(s, sizeof(s), "%s", job_state_text(j));
    body_draw(s, x + (w - body_width(s)) / 2, y + 168, C_TEXT);
    frame_rect(x + 40, y + 198, w - 80, 40, 4, C_WHITE);
    seg_bar(x + 50, y + 208, w - 100, 20, k, C_WHITE, RGBA8(0x24, 0x24, 0x24, 0xFF));
    char pc[8];
    snprintf(pc, sizeof(pc), "%d%%", (int)(k * 100));
    title_draw(pc, x + (w - title_width(pc, 1)) / 2, y + 246, 1, C_SOFT);
    const char *n = TXT(S_UPD_RESTART);
    body_draw(n, x + (w - body_width(n)) / 2, y + 272, C_MUTED);
    char sk[64];
    snprintf(sk, sizeof(sk), "%s", TXT(S_UPD_SKIP));
    float sw = body_width(sk) + 22;
    btn_icon(1, x + (w - sw) / 2, y + 307);
    body_draw(sk, x + (w - sw) / 2 + 22, y + 301, C_WHITE);
}

/* ================================================================ main */

int main(void) {
    for (int l = 0; l < 2; l++)
        for (int k = 0; k < S_COUNT; k++) utf8_to_latin1(UI_UTF8[l][k], UI[l][k], sizeof(UI[l][k]));

    vita2d_init();
    vita2d_set_clear_color(C_BG);
    vita2d_set_vblank_wait(1);
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);

    gfx_init();
    audio_init();
    settings_load();
    reg_load();
    sceSysmoduleLoadModule(SCE_SYSMODULE_IME);
    SceCommonDialogConfigParam cdc;
    sceCommonDialogConfigParamInit(&cdc);
    sceCommonDialogSetConfigParam(&cdc);
    /* catálogo: o último sincronizado (cartão) ou o que veio no VPK */
    int cached = store_load_cache(g_posts, MAX_POSTS, &app_rel);
    first_sync = cached <= 0;
    if (cached > 0) g_nposts = cached;
    else catalog_load("app0:data/catalog.txt");
    feat = catalog_featured();
    load_covers();
    make_mosaic();
    /* Defer helper removal only until it has finished exiting, not until a
     * SECOND launch of Store. The updater writes the marker after promotion. */
    fs_mkdir_p(WS_DATA);
    int upgraded_this_boot = 1;
    {
        int last_len = 0;
        char *last = fs_read(WS_DATA "/last_started_version.txt", &last_len);
        if (last) {
            upgraded_this_boot = strcmp(last, WS_VERSION) != 0;
            free(last);
        }
        fs_write(WS_DATA "/last_started_version.txt", WS_VERSION, (int)strlen(WS_VERSION));
    }
    updater_cleanup_pending =
        fs_exists("ux0:app/WFFS00043/sce_sys/param.sfo") &&
        (upgraded_this_boot || fs_exists(WS_DATA "/updater_cleanup.pending"));
    if (updater_cleanup_pending)
        ws_log("updater: deferred cleanup scheduled AFTER Home is ready");
    /* A v0.99 bridge installs WOLF00002, but this app uses WFFS00043.
     * Only remove a legacy bubble whose installed SFO explicitly proves
     * it is OUR updater. Never delete the old WOLF00001 main app. */
    {
        char title[128] = "";
        legacy_updater_cleanup_pending =
            fs_exists(WS_DATA "/updater_cleanup.pending") &&
            fs_exists("ux0:app/WOLF00002/sce_sys/param.sfo") &&
            sfo_get("ux0:app/WOLF00002/sce_sys/param.sfo",
                    "TITLE", title, sizeof(title)) == 0 &&
            strcmp(title, "Wolff's Store Updater") == 0;
        if (legacy_updater_cleanup_pending)
            ws_log("updater: verified legacy helper cleanup scheduled");
    }
    /* sobras de uma atualização do próprio app */
    fs_rm_rf(WS_PKG_SELF);
    fs_rm_rf(WS_PKG_UPD);
    jobs_init();
    start_sync(0);

    double t_start = clock_s();
    now_s = 0;
    view_t0 = 0;
    build_home();
    if (!first_sync) title_cues();

    unsigned prev = 0;
    int prev_ly = 128;
    double rep_t = 0;
    int rep_dir = -1;
    int touching = 0;
    float t_x0 = 0, t_y0 = 0, t_last = 0, t_cam0 = 0;
    double t_t0 = 0;
    int dragged = 0;
    int music_started_after_frame = 0;

    for (;;) {
        now_s = clock_s() - t_start;
        /* Vita RTC is local: select the daypart automatically and refresh
         * only occasionally, including when the console clock changes. */
        static double ost_clock_next = 0;
        if (now_s >= ost_clock_next) {
            audio_ost_refresh_clock();
            ost_clock_next = now_s + 20.0;
        }
        SceCtrlData pad;
        memset(&pad, 0, sizeof(pad));
        sceCtrlPeekBufferPositive(0, &pad, 1);
        unsigned b = pad.buttons, down = b & ~prev;
        prev = b;

        /* analógico esquerdo funciona como direcional */
        if (pad.ly < 40) b |= SCE_CTRL_UP;
        if (pad.ly > 216) b |= SCE_CTRL_DOWN;
        if (pad.lx < 40) b |= SCE_CTRL_LEFT;
        if (pad.lx > 216) b |= SCE_CTRL_RIGHT;

        if (first_sync) {
            if (down & (SCE_CTRL_START | SCE_CTRL_CROSS)) {
                bark_t0 = now_s;
                letters_wave(now_s, 1);
                sfx("bark");
            }
        } else if (!started) {
            if (down & (SCE_CTRL_START | SCE_CTRL_CROSS | SCE_CTRL_CIRCLE)) start_intro();
        } else if (now_s - intro_t0 < T_DONE) {
            /* ✕ ou START durante a intro: pula para o final */
            if (down & (SCE_CTRL_START | SCE_CTRL_CROSS)) {
                intro_t0 = now_s - T_DONE;
                ncues = cue_next = 0;
                wave_t0 = -100;
                rebuild();
                for (int i = 0; i < nitems; i++)
                    if (items[i].act == A_POST && items[i].arg == feat && items[i].h <= 96) focus = i;
            }
        } else if (ime_open) {
            /* o teclado do sistema cuida da entrada */
        } else if (vid_open) {
            if (down & SCE_CTRL_CIRCLE) close_video();
            if (down & SCE_CTRL_CROSS) video_toggle_pause();
            if (down & (SCE_CTRL_RIGHT | SCE_CTRL_RTRIGGER)) video_seek(10);
            if (down & (SCE_CTRL_LEFT | SCE_CTRL_LTRIGGER)) video_seek(-10);
        } else if (snd_menu) {
            if (down & SCE_CTRL_LEFT && snd_row > 0) { snd_row = 0; sfx("hover"); }
            if (down & SCE_CTRL_RIGHT && snd_row < 1) { snd_row = 1; sfx("hover"); }
            if (down & SCE_CTRL_CROSS) snd_menu_toggle(snd_row);
            if (down & (SCE_CTRL_CIRCLE | SCE_CTRL_UP)) { snd_menu = 0; sfx("back"); }
        } else if (settings_open) {
            int dir = (down & SCE_CTRL_UP) ? 0 : (down & SCE_CTRL_DOWN) ? 1 : -1;
            if (pad.ly < 40 && !(prev_ly < 40)) dir = 0;
            if (pad.ly > 216 && !(prev_ly > 216)) dir = 1;
            if (dir == 0 && settings_row > 0) { settings_row--; sfx("hover"); }
            if (dir == 1 && settings_row < settings_rows() - 1) { settings_row++; sfx("hover"); }
            if (down & SCE_CTRL_LEFT) settings_change(-1);
            if (down & SCE_CTRL_RIGHT) settings_change(1);
            if (down & SCE_CTRL_CROSS) settings_change(0);
            if (down & SCE_CTRL_CIRCLE) settings_back();
            if (down & SCE_CTRL_START) close_settings();
        } else if (upd_job >= 0 && job_by_id(upd_job) && job_by_id(upd_job)->state < J_DONE) {
            /* atualização do app na tela: ○ deixa para depois */
            if (down & SCE_CTRL_CIRCLE) {
                jobs_cancel(upd_job);
                upd_job = -1;
                upd_skipped = 1;
                sfx("back");
            }
        } else if (viewer >= 0) {
            if (down & (SCE_CTRL_CIRCLE | SCE_CTRL_CROSS)) back();
            if (down & (SCE_CTRL_RTRIGGER | SCE_CTRL_RIGHT) && shot_count) {
                viewer = (viewer + 1) % shot_count;
                sfx("tick");
            }
            if (down & (SCE_CTRL_LTRIGGER | SCE_CTRL_LEFT) && shot_count) {
                viewer = (viewer + shot_count - 1) % shot_count;
                sfx("tick");
            }
        } else {
            int dir = (b & SCE_CTRL_UP) ? 0 : (b & SCE_CTRL_DOWN) ? 1 : (b & SCE_CTRL_LEFT) ? 2 : (b & SCE_CTRL_RIGHT) ? 3 : -1;
            if (dir >= 0) {
                if (dir != rep_dir) {
                    move(dir);
                    rep_dir = dir;
                    rep_t = now_s + .35;
                } else if (now_s >= rep_t) {
                    move(dir);
                    rep_t = now_s + .12;
                }
            } else {
                rep_dir = -1;
            }
            if (down & SCE_CTRL_CROSS) activate(focus);
            if (down & SCE_CTRL_CIRCLE) back();
            if (down & SCE_CTRL_TRIANGLE) start_sync(1);      /* sincronizar com o repo */
            if (down & SCE_CTRL_SQUARE) {                       /* downloads */
                if (view == V_DLS) back();
                else {
                    sfx("open");
                    go(V_DLS, 0);
                }
            }
            if (down & SCE_CTRL_SELECT) open_search();          /* pesquisa */
            if (down & SCE_CTRL_START) open_settings();         /* configurações */
            if (down & SCE_CTRL_LTRIGGER) {
                cam_target -= SCR_H - 120;
                cam_clamp();
            }
            if (down & SCE_CTRL_RTRIGGER) {
                cam_target += SCR_H - 120;
                cam_clamp();
            }
            /* analógico direito rola a página */
            int ry = pad.ry - 128;
            if (ry > 24 || ry < -24) {
                cam_target += ry / 10.f;
                cam_clamp();
            }
        }

        /* toque: arrastar rola, tocar abre */
        SceTouchData td;
        memset(&td, 0, sizeof(td));
        sceTouchPeek(SCE_TOUCH_PORT_FRONT, &td, 1);
        if (td.reportNum > 0) {
            float tx = td.report[0].x / 2.f, ty = td.report[0].y / 2.f;
            if (!touching) {
                touching = 1;
                dragged = 0;
                t_x0 = tx;
                t_y0 = ty;
                t_cam0 = cam_target;
                t_t0 = now_s;
            } else if (fabsf(ty - t_y0) > 16 || fabsf(tx - t_x0) > 16) {
                dragged = 1;
            }
            /* passar o dedo no logo: as letras pulam com a nota (como o hover no PC) */
            if ((started || first_sync) && !vid_open && !settings_open) {
                float wy = ty + cam;
                float logo_x = first_sync ? (SCR_W - title_width(LOGO, 5)) / 2 : LOGO_X;
                float logo_y = first_sync ? SCR_H / 2 - 52 : 30;
                float logo_step = first_sync ? 40 : LOGO_STEP;
                if (wy >= logo_y && wy < logo_y + 56 && tx >= logo_x && tx < logo_x + 13 * logo_step) {
                    int li = (int)((tx - logo_x) / logo_step);
                    if (li >= 0 && li < 13 && LOGO[li] != ' ' && now_s - letter_hop[li] > .2) {
                        letter_hop[li] = now_s;
                        sfx_letter(li);
                    }
                }
            }
            if (dragged && viewer < 0 && !vid_open && !settings_open && !snd_menu) {
                cam_target = t_cam0 - (ty - t_y0);
                cam_clamp();
                cam = cam_target;
            }
            t_last = ty;
        } else if (touching) {
            touching = 0;
            if (!dragged && now_s - t_t0 < .6) {
                if (first_sync) {
                    bark_t0 = now_s;
                    letters_wave(now_s, 1);
                    sfx("bark");
                } else if (!started) start_intro();
                else if (vid_open) video_toggle_pause();
                else if (snd_menu_touch(t_x0, t_y0)) {
                } else if (t_x0 >= 24 && t_x0 < 24 + 72 && t_y0 + cam >= 12 && t_y0 + cam < 12 + 76 && !settings_open) {
                    if (view != V_HOME) go(V_HOME, 0);
                    else {
                        bark_t0 = now_s; /* na Home, tocar na pata: latido */
                        sfx("bark");
                    }
                } else if (ime_open) {
                } else if (settings_open) {
                    float x, y, w, h, rh;
                    settings_geometry(&x, &y, &w, &h, &rh);
                    int row = (int)((t_y0 - (y + 90)) / rh);
                    if (t_x0 < x || t_x0 > x + w || t_y0 < y || t_y0 > y + h) settings_back();
                    else if (t_y0 >= y + 90 && row >= 0 && row < settings_rows()) {
                        settings_row = row;
                        settings_change(settings_page == SETTINGS_ROOT ? 0 : (t_x0 < x + w / 2 ? -1 : 1));
                    }
                } else if (viewer >= 0) back();
                else {
                    float px = t_x0, py = t_y0 + cam;
                    for (int i = nitems - 1; i >= 0; i--) {
                        Item *it = &items[i];
                        if (px >= it->x && px < it->x + it->w && py >= it->y && py < it->y + it->h && it->pop <= now_s) {
                            focus = i;
                            activate(i);
                            break;
                        }
                    }
                }
            }
        }
        (void)t_last;

        prev_ly = pad.ly;
        poll_ime();
        run_cues();
        tick_store();
        {
            float nh = 36; /* notifications reuse unused right side of hints */
            if (nh != hint_h) {
                hint_h = nh;
                cam_follow();
                cam_clamp();
            }
        }
        if (fabsf(cam_target - cam) > 1) scroll_t = now_s;
        cam += (cam_target - cam) * .25f;
        if (fabsf(cam_target - cam) < .5f) cam = cam_target;

        vita2d_set_clear_color(started ? C_BG : RGBA8(0, 0, 0, 0xFF));
        vita2d_start_drawing();
        vita2d_clear_screen();
        if (first_sync) {
            draw_first_sync();
        } else if (!started) {
            draw_title_screen(now_s - title_t0, 0);
        } else {
            double t = now_s - intro_t0;
            float save = cam;
            cam = (float)(int)cam;
            draw_page();
            cam = save;
            /* a pergunta some com o efeito de TV desligando (.3 s, 5 passos) */
            if (t < .3) draw_title_screen(now_s - title_t0, ((int)(t / .06) + 1) / 5.f);
            if (viewer >= 0) draw_viewer();
            if (now_s - intro_t0 >= T_DONE) {
                draw_update();
            }
            draw_snd_menu();
            draw_settings();
            float dy, al;
            if (pop_state(intro_t0 + T_UI, &dy, &al)) {
                if (viewer < 0) draw_scrollbar(al);
                hint_slide = 36 * (1 - al);
                float hint_right = draw_hints();
                if (now_s - intro_t0 >= T_DONE) draw_strip(hint_right);
            }
        }
        draw_video();
        /* transição wr-wipe: sólido, 75%, 50%, 25% */
        double we = now_s - wipe_t0;
        if (we < .4) dither_fill((int)(we / .1), 0, 0, SCR_W, SCR_H, C_BG);
        /* brilho do app: escurece tudo por cima */
        if (brightness < 100) vita2d_draw_rectangle(0, 0, SCR_W, SCR_H, RGBA8(0, 0, 0, (100 - brightness) * 0xC0 / 60));
        vita2d_end_drawing();
        if (ime_open) vita2d_common_dialog_update(); /* teclado do sistema por cima */
        vita2d_swap_buffers();
        /* Start BGM only AFTER the fully loaded main Home has been drawn.
         * No music while loading, on the title/intro, or during handoff. */
        if (!music_started_after_frame && !first_sync && started &&
            view == V_HOME && now_s - intro_t0 >= T_DONE &&
            upd_job < 0) {
            audio_start_music_after_first_frame();
            music_started_after_frame = 1;
            ws_log("music: started after main Home became ready");
        }
    }
    return 0;
}
