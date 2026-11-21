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
#include <ctype.h>
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
#include "polls.h"
#include "vpk_version.h"
#include "sync.h"

#ifndef WS_VERSION
#define WS_VERSION "1.0.14"
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
    S_VID_FAIL, S_VID_HINT, S_BARK, S_WATCH_APP, S_VIDEO_NOTE_APP, S_LATEST3, S_TROPHIES, S_OPEN_APP, S_LBL_SYNC, S_LBL_DLS, S_LBL_AUDIO, S_LBL_SETTINGS, S_LBL_SEARCH, S_POST_PUBLISHED, S_POST_UPDATED, S_APP_LIVEAREA,
    S_TRANS_DIALOG_TITLE, S_TRANS_REQUIRE, S_TRANS_DETECTED, S_TRANS_MISSING,
    S_TRANS_CONFIRM, S_TRANS_CANCEL, S_TRANS_INSTALLING, S_RESOLVING_LINK, S_CONNECTING,
    S_CAP_TITLE, S_CAP_RECOMMEND, S_CAP_CPU, S_CAP_OPTIONAL, S_COUNT
};

static const char *UI_UTF8[2][S_COUNT] = {
    [LANG_EN] = {
        "Ports", "Apps/Mods", "Translations", "Journal",
        "In development", "Completed", "Upcoming", "Other teams",
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
        "No downloads yet. Open a project and pick a download.", "Syncing", "Catalog updated",
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
        "The video plays right here in the app.", "Check out the latest updates...", "Native trophies", "Open", "Sync", "Downloads", "Audio", "Settings", "Search",
        "Published on", "Last update",
        "After installation: refresh LiveArea with FMVita or File Manager.",
        "INSTALL TRANSLATION", "This translation requires the rePatch plugin!",
        "Plugin detected in taiHEN.", "Plugin missing from taiHEN.",
        "CONFIRM", "CANCEL", "Installing the translation.",
        "Resolving link...", "Connecting...",
        "RECOMMENDED PLUGIN", "Recommended: ", "CapUnlocker enables all four CPU cores.",
        "Optional. The app can be installed without this plugin.",
    },
    [LANG_PT] = {
        "Ports", "Apps/Mods", "Traduções", "Journal",
        "Em desenvolvimento", "Finalizados", "Próximos", "Outras equipes",
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
        "Sincronizando", "Catálogo atualizado", "Sem internet: usando o catálogo salvo",
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
        "O vídeo toca aqui mesmo no app.", "Confira as últimas novidades...", "Troféus nativos", "Abrir", "Sincronizar", "Downloads", "Áudio", "Configurações", "Pesquisar",
        "Publicado em", "Atualizado em",
        "Após instalar, atualize a LiveArea pelo FMVita ou File Manager.",
        "INSTALAR TRADUÇÃO", "Essa tradução exige o plugin rePatch instalado!",
        "Plugin detectado no taiHEN.", "Plugin ausente no taiHen.",
        "CONFIRMAR", "CANCELAR", "Instalando tradução.",
        "Resolvendo link...", "Conectando...",
        "PLUGIN RECOMENDADO", "Recomendado: ", "CapUnlocker libera o uso dos quatro núcleos da CPU.",
        "Opcional. A instalação também funciona sem esse plugin.",
    },
};
static char UI[2][S_COUNT][128];

static int lang = LANG_EN;
#define TXT(k) (UI[lang][k])

/* ================================================================ estado */

enum { V_HOME, V_CAT, V_POST, V_DLS, V_SEARCH };
#define BADGE 52 /* botões do topo */
static int view = V_HOME, view_cat = 0, view_post = -1, from_view = V_HOME, peek = -1;
/* -1 means All; filters belong to each category and survive post navigation. */
static int cat_filter[CAT_COUNT] = {-1, -1, -1, -1};

/* Visual chronology starts as 4 / 3 / 1 / 2 / 5, front card in slot 2.
 * Array indices refer to the CURRENT synced catalog: no duplicate textures. */
#define DECK_LIMIT 12
static int deck_order[DECK_LIMIT], deck_rank[DECK_LIMIT];
static int deck_n = 0, deck_pos = 2, deck_last_count = 0;
static double deck_access_t0 = 1e9; /* prevent clicks and D-pad before intro */
static double deck_moved_t0 = -100;
static int deck_direction = 0;
static int dls_from = V_HOME, dls_from_arg = 0;
static float hint_h = 36; /* barra de dicas (+ faixa de status quando aparece) */
static int brightness = 100;  /* % (escurece a imagem do app; 40..100) */
static int settings_open = 0, settings_row = 0, settings_page = 0, settings_root_row = 0;
static int poll_selected = -1;
static double poll_anim_t0 = -100;
static int poll_anim_post = -1;
/* Page position of the ballot box, directly below the confirm button. */
static float poll_box_page_y = -1.f;
static vita2d_texture *poll_box_tex = NULL, *poll_paper_tex = NULL;
static unsigned poll_seen_version = 0;
static int trans_confirm_post = -1, trans_confirm_dl = -1;
static int trans_confirm_mod = 0, trans_confirm_backup = 0, trans_confirm_plugin = 0;
static int trans_confirm_app = 0, trans_confirm_base_missing = 0;
static int plugin_advice_post = -1, plugin_advice_dl = -1;
static double settings_t0 = -10;
static char search_q[128] = "";
static int ime_open = 0;
static int sync_manual = 0, sync_reported = 1;
static AppRelease app_rel;
static int upd_job = -1, upd_skipped = 0; /* atualização do próprio app */
static int update_notes_pending = 0, update_notes_visible = 0;
static double update_notes_t0 = -100;
static double letter_hop[16] = {-100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100};
static double bark_t0 = -100, sync_done_t = -100;
static int snd_menu = 0, snd_row = 0, vid_open = 0, vid_music = 0;
static double now_s, view_t0, peek_t0, intro_t0 = -1, wipe_t0 = -100, wave_t0 = -100, pressed_t0 = -100;
static int started = 0, pressed_item = -1;
static int first_sync = 0;
static double title_t0 = 0, first_sync_t0 = 0;
static double app_clock_t0 = 0; /* monotonic origin for post-sync typewriter */
static double home_t0 = -100; /* animação de volta ao início */
static int feat = 0;
/* Track the latest news post by ID, not just date: two posts may share a day. */
static char journal_seen_slug[48] = "";
static const char *journal_latest_slug(void) {
    for (int i = 0; i < g_nposts; i++)
        if (g_posts[i].cat == CAT_REVIEWS && g_posts[i].subcat == SUB_NEWS)
            return g_posts[i].slug;
    return "";
}
static int journal_has_unread_news(void) {
    const char *latest = journal_latest_slug();
    return latest[0] && strcmp(latest, journal_seen_slug) != 0;
}
static void journal_mark_seen(void) {
    const char *latest = journal_latest_slug();
    if (!latest[0]) return;
    snprintf(journal_seen_slug, sizeof(journal_seen_slug), "%s", latest);
    fs_write(WS_DATA "/journal_seen_slug.txt", journal_seen_slug,
             (int)strlen(journal_seen_slug));
}

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
       OP_BADGE, OP_DLJOB, OP_JOBROW, OP_WARN, OP_HLABEL, OP_ICON, OP_TROPHY, OP_DECK_CARD };
enum { A_NONE, A_POST, A_PEEK, A_ALL, A_HOME, A_BACK, A_URL, A_SHOT, A_TOP, A_START,
       A_SYNC, A_DLS, A_LANG, A_SOUND, A_DL, A_CANCEL, A_CLEAR, A_SETTINGS, A_SEARCH, A_SEARCHBOX, A_REGUPD, A_VIDEO, A_LAUNCH,
       A_POLL_CHOOSE, A_POLL_SEND, A_CAT_FILTER, A_DECK_CARD, A_DECK_ARROW };

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

/* Site-style square control for the other releases deck. A native pixel
 * triangle avoids the double "<</>>" glyph and text-width inconsistencies. */
static int deck_arrow(float x, float y, int direction, double pop) {
    const float w = 48.f, h = 48.f;
    int it = item(x, y, w, h, A_DECK_ARROW, direction, NULL);
    if (it >= 0) items[it].pop = pop;
    Op *bg = rect(x, y, w, h, C_DARK);
    bg->item = it;
    bg->pop = pop;
    Op *fr = frame(x, y, w, h, C_BTNBRD, C_WHITE, it);
    fr->pop = pop;
    Op *arrow = op(OP_TRI, x + 16, y + 15, 16, 16);
    arrow->scale = direction > 0 ? 1 : 0;
    arrow->c = arrow->cf = C_WHITE;
    arrow->item = it;
    arrow->pop = pop;
    Op *glow = halo(x, y, w, h, 0, 0, 1, it);
    glow->pop = pop;
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
                /* Prefer the version registered by a successful Store VPK
                 * install when the package's APP_VER was not bumped. Verify
                 * matching TitleID/path first; otherwise keep real SFO. */
                const char *installed_version = ws_vpk_installed_version(
                    av,
                    r && r->kind == DL_VPK ? r->version : NULL,
                    r && r->kind == DL_VPK ? r->path : NULL,
                    t);
                snprintf(s->inst_ver, sizeof(s->inst_ver), "%s", installed_version);
            }
        }
    } else if (d->type == DL_TRANS) {
        for (int i = 0; i < d->ntids && !s->installed; i++) {
            char path[64], game[96];
            snprintf(path, sizeof(path), "ux0:rePatch/%s", d->tids[i]);
            s->installed = fs_is_dir(path);
            if (s->installed) {
                snprintf(game, sizeof(game), "ux0:app/%s/sce_sys/param.sfo", d->tids[i]);
                if (fs_exists(game)) snprintf(s->tid, sizeof(s->tid), "%s", d->tids[i]);
            }
        }
        if (s->installed && r) snprintf(s->inst_ver, sizeof(s->inst_ver), "%s", r->version);
    } else if (d->type == DL_TAI_PLUGIN) {
        s->installed=fs_exists("ur0:tai/DDTouchUI.suprx");
        if(s->installed && r)snprintf(s->inst_ver,sizeof(s->inst_ver),"%s",r->version);
    } else if (d->type == DL_APP_FILES) {
        /* O jogo base instalado não equivale à tradução aplicada. */
        if (r && d->ntids > 0 && fs_is_dir(r->path)) {
            char sfo[96];
            snprintf(sfo, sizeof(sfo), "ux0:app/%s/sce_sys/param.sfo", d->tids[0]);
            if (fs_exists(sfo)) {
                s->installed = 1;
                snprintf(s->tid, sizeof(s->tid), "%s", d->tids[0]);
                snprintf(s->inst_ver, sizeof(s->inst_ver), "%s", r->version);
            }
        }
    } else if (r && fs_exists(r->path)) {
        s->installed = 1;
        snprintf(s->inst_ver, sizeof(s->inst_ver), "%s", r->version);
    }
    if (s->installed && d->version[0] && s->inst_ver[0]) s->update = version_cmp(s->inst_ver, d->version) < 0;
}

/* plugin rePatch listado no config do taiHEN */
static int repatch_from_config(const char *config) {
    char *text = fs_read(config, NULL);
    if (!text) return 0;
    int found = 0;
    char *save = NULL;
    for (char *line = strtok_r(text, "\n", &save); line;
         line = strtok_r(NULL, "\n", &save)) {
        while (*line && isspace((unsigned char)*line)) line++;
        if (!*line || *line == '#' || *line == ';' || *line == '*') continue;
        char *end = line;
        while (*end && !isspace((unsigned char)*end) && *end != '#') end++;
        *end = 0;
        char path[260];
        snprintf(path, sizeof(path), "%s", line);
        for (char *c = path; *c; c++) *c = (char)tolower((unsigned char)*c);
        if (strstr(path, "repatch") && (strstr(path, ".skprx") || strstr(path, ".suprx"))
            && fs_exists(line)) { found = 1; break; }
    }
    free(text);
    return found;
}

static int repatch_plugin(void) {
    return repatch_from_config("ur0:tai/config.txt") ||
           repatch_from_config("ux0:tai/config.txt");
}

static void open_translation_confirm(int pi, int di) {
    if (pi < 0 || pi >= g_nposts || di < 0 || di >= g_posts[pi].ndls) return;
    const Download *dl = &g_posts[pi].dls[di];
    if (dl->type != DL_TRANS && dl->type != DL_APP_FILES) return;
    trans_confirm_post = pi;
    trans_confirm_dl = di;
    trans_confirm_mod = 0;
    trans_confirm_backup = 0;
    trans_confirm_app = dl->type == DL_APP_FILES;
    trans_confirm_base_missing = 0;
    trans_confirm_plugin = trans_confirm_app ? 1 : repatch_plugin();
    if (trans_confirm_app) {
        char game[96];
        snprintf(game, sizeof(game), "ux0:app/%s", dl->ntids ? dl->tids[0] : "?");
        trans_confirm_base_missing = !dl->ntids || !fs_is_dir(game);
        sfx("notice");
        return;
    }
    for (int i = 0; i < dl->ntids; i++) {
        char dir[80], bak[96];
        snprintf(dir, sizeof(dir), "ux0:rePatch/%s", dl->tids[i]);
        snprintf(bak, sizeof(bak), "%s_bkp", dir);
        if (fs_exists(dir)) trans_confirm_mod = 1;
        if (fs_exists(dir) && fs_exists(bak)) trans_confirm_backup = 1;
    }
    sfx("open");
}

static void open_plugin_advice(int pi, int di) {
    if (pi < 0 || pi >= g_nposts || di < 0 || di >= g_posts[pi].ndls) return;
    const Download *d = &g_posts[pi].dls[di];
    if(d->type != DL_TAI_PLUGIN &&
       (d->type != DL_VPK || !d->advisory_plugin || !d->advisory_plugin[0]))return;
    plugin_advice_post = pi;
    plugin_advice_dl = di;
    sfx("notice");
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
    if (p->poll_id[0]) {
        const char *chip=poll_local_answered(p->poll_id)?
            (lang==LANG_PT?"RESPONDIDA":"ANSWERED"):
            (lang==LANG_PT?"ENQUETE":"POLL");
        float cw=body_width(chip)+22;
        rect(x+12,y+12,cw,28,RGBA8(0x06,0x06,0x06,0xE6));
        frame(x+12,y+12,cw,28,C_GOLD,C_GOLD,-1)->bold=2;
        body(chip,x+23,y+13,C_WHITE)->bold=1;
    }
    if (p->ndls) {
        /* etiqueta no canto da capa: atualização disponível ou o tipo do download */
        const char *chip = p->dls[0].type == DL_TRANS ? TXT(S_CHIP_TRANS) : (p->dls[0].type == DL_PLUGIN || p->dls[0].type == DL_TAI_PLUGIN) ? "Plugin" : "VPK";
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

static int featured_post_for_device(void) {
    int chosen=catalog_featured();
    if(chosen>=0 && chosen<g_nposts &&
       g_posts[chosen].poll_id[0] && poll_local_answered(g_posts[chosen].poll_id)) {
        for(int i=0;i<g_nposts;i++)
            if(!g_posts[i].poll_id[0] && g_posts[i].news_kind) return i;
    }
    return chosen;
}

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
    /* Last three updates use the traditional large cards. The deck below
     * displays ONLY older entries (4th onward), with no repeated posts. */
    y = sy + sh + 44;
    int latest_ids[3], latest_n = 0;
    int older_ids[DECK_LIMIT];
    deck_n = 0;
    for (int i = 0; i < g_nposts; i++) {
        if (g_posts[i].poll_id[0] || g_posts[i].cat == CAT_REVIEWS) continue;
        if (latest_n < 3) latest_ids[latest_n++] = i;
        else if (deck_n < DECK_LIMIT) older_ids[deck_n++] = i;
        if (latest_n == 3 && deck_n == DECK_LIMIT) break;
    }
    if (latest_n) {
        int heading_first = nops;
        y += heading(TXT(S_LATEST3), 24, y, 912, 2, C_SOFT, 0, 1, 1, 24) + 24;
        for (int i = heading_first; i < nops; i++) ops[i].pop = nav_pop(0);
        y += grid(latest_ids, latest_n, y, nav_pop(0) + .05, .08) + 44;
    }

    /* Stored chronologically, so arrows advance 01 -> 02 -> 03.
     * Only the first five-card layout uses the special visual fan
     * 4 / 3 / 1 / 2 / 5; this does not shuffle the navigation order. */
    for (int i = 0; i < deck_n; ++i) {
        deck_order[i] = older_ids[i];
        deck_rank[i] = i + 1;
    }
    if (deck_n != deck_last_count || deck_pos >= deck_n) {
        deck_pos = 0;
        deck_last_count = deck_n;
    }
    deck_access_t0 = 1e9;
    if (deck_n) {
        /* The whole section begins only AFTER the first Home intro (T_DONE).
         * Do not let the deck/arrow/counter render or become touchable while
         * the logo/typewriter/CRT animation is running. Returning Home uses
         * the shorter existing re-entry timeline instead of replaying intro. */
        const double others_pop = home_pop(T_DONE + .12, .72);
        deck_access_t0 = others_pop;
        int heading_first = nops;
        char release_heading[48];
        utf8_to_latin1(lang == LANG_PT ? "Lançamentos" : "Releases",
                       release_heading, sizeof(release_heading));
        y += heading(keep(release_heading),
                     24, y, 912, 2, C_SOFT, 0, 1, 1, 24) + 22;
        for (int i = heading_first; i < nops; i++) ops[i].pop = others_pop;

        const int visual[5] = {-2, 2, -1, 1, 0}; /* paint center LAST */
        const int small4[4] = {-2, 1, -1, 0};
        const int small3[3] = {-1, 1, 0};
        const int small2[2] = {1, 0};
        for (int v = 0; v < (deck_n >= 5 ? 5 : deck_n); ++v) {
            const int slot = deck_n >= 5 ? visual[v] :
                             deck_n == 4 ? small4[v] :
                             deck_n == 3 ? small3[v] :
                             deck_n == 2 ? small2[v] : 0;
            int di = (deck_pos + slot + deck_n) % deck_n;
            if (deck_pos == 0 && deck_n >= 5) {
                /* At first view, positions are 4,3,1,2,5. */
                if (slot == -2) di = 3;
                else if (slot == -1) di = 2;
                else if (slot == 0) di = 0;
                else if (slot == 1) di = 1;
                else if (slot == 2) di = 4;
            }
            const int pi = deck_order[di];
            float bx, by, bw, bh;
            switch (slot) {
            case -2: bx = 62; by = y + 56; bw = 188; bh = 222; break;
            case  2: bx = 710; by = y + 56; bw = 188; bh = 222; break;
            case -1: bx = 208; by = y + 35; bw = 248; bh = 264; break;
            case  1: bx = 504; by = y + 35; bw = 248; bh = 264; break;
            default: bx = 332; by = y + 10; bw = 296; bh = 306; break;
            }
            int it = item(bx, by, bw, bh, A_DECK_CARD, di, NULL);
            if (it >= 0) items[it].pop = others_pop + .04;
            Op *o = op(OP_DECK_CARD, bx, by, bw, bh);
            o->scale = pi;
            o->bold = slot + 2;
            o->item = it;
            o->pop = others_pop + .04;
        }
        y += 338;
        /* Square pixel-arrow buttons: same 4px borders, 48px height and
         * grayscale focus state as the site. */
        deck_arrow(340, y, -1, others_pop + .08);
        char counter[24];
        snprintf(counter, sizeof(counter), "%02d / %02d",
                 deck_rank[deck_pos], deck_n);
        int counter_first = nops;
        title(keep(counter), (SCR_W - title_width(counter, 2)) / 2,
              y + 16, 2, C_SOFT, 0);
        for (int i = counter_first; i < nops; i++)
            ops[i].pop = others_pop + .08;
        deck_arrow(572, y, 1, others_pop + .08);
        y += 48 + 32;
        int caption_first = nops;
        y += para(lang == LANG_PT
                 ? "Explore outros trabalhos finalizados ou em desenvolvimento..."
                 : "Explore other completed and in-development projects...",
                 24, y, 912, C_SOFT, 1, 2, 0) + 24;
        for (int i = caption_first; i < nops; i++)
            ops[i].pop = others_pop + .12;
    }

    /* categorias */
    float bw = (912 - 3 * 32) / 4.f;
    for (int c = 0; c < CAT_COUNT; c++) {
        float bx = 24 + c * (bw + 32);
        int it = item(bx, y, bw, 56, A_PEEK, c, NULL);
        if (it >= 0) items[it].pop = nav_pop(c);
        int sel = peek == c;
        int journal_alert = c == CAT_REVIEWS && journal_has_unread_news();
        Op *o = rect(bx, y, bw, 56, sel ? C_WHITE : C_BTN);
        o->item = it;
        o->pop = nav_pop(c);
        o = frame(bx, y, bw, 56, sel ? C_WHITE : C_BTNBRD, C_WHITE, it);
        o->pop = nav_pop(c);
        int f0 = nops;
        heading(TXT(S_CAT0 + c), bx + 4, y + 20, bw - 8, 2,
                sel ? C_DARK : journal_alert ? C_GOLD : C_WHITE, 0, 1, 1, 24);
        for (int i = f0; i < nops; i++) {
            if (journal_alert && !sel && ops[i].type == OP_TITLE)
                ops[i].bold = 3; /* special subtle gold pulse */
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

/* Preserve the original post groups, but display Finalizados first. */
static const int cat_group_order[4] = {GRP_DONE, GRP_DOING, GRP_NEXT, GRP_OTHERS};
static const int cat_apps_order[4] = {SUB_APP, SUB_TOOL, SUB_MOD, SUB_PLUGIN};
static const int cat_journal_order[3] = {SUB_NEWS, SUB_REVIEW, SUB_POLL};

static int cat_section_count(int c) {
    return c == CAT_REVIEWS ? 3 : 4;
}
static int cat_section_key(int c, int slot) {
    if (slot < 0 || slot >= cat_section_count(c)) return -1;
    if (c == CAT_TOOLS) return cat_apps_order[slot];
    if (c == CAT_REVIEWS) return cat_journal_order[slot];
    return cat_group_order[slot];
}
static int cat_post_in_section(int c, int section, const Post *p) {
    int key = cat_section_key(c, section);
    return key >= 0 && p->cat == c &&
           ((c == CAT_TOOLS || c == CAT_REVIEWS) ? p->subcat == key : p->group == key);
}
static const char *cat_section_label(int c, int section) {
    static const char *en[] = {"Apps", "Tools", "Mods", "Plugins", "News", "Reviews", "Polls"};
    static const char *pt[] = {"Apps", "Ferramentas", "Modificações", "Plugins", "Notícias", "Reviews", "Enquetes"};
    int key = cat_section_key(c, section);
    if (key < 0) return "";
    if (c == CAT_TOOLS || c == CAT_REVIEWS) {
        char tmp[96];
        utf8_to_latin1(lang == LANG_PT ? pt[key] : en[key], tmp, sizeof(tmp));
        return keep(tmp);
    }
    return TXT(S_GRP0 + key);
}

/* Compact labels ONLY on the "Ver somente" controls. Category headings
 * preserve their full names, while the clickable filter tiles stay equal. */
static const char *cat_filter_label(int c, int section) {
    int key = cat_section_key(c, section);
    if (c == CAT_TOOLS && key == SUB_MOD) return "Mods";
    if (c != CAT_TOOLS && c != CAT_REVIEWS) {
        if (key == GRP_DOING) return lang == LANG_PT ? "Andamento" : "Progress";
        if (key == GRP_OTHERS) return lang == LANG_PT ? "Terceiros" : "Third-party";
    }
    return cat_section_label(c, section);
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
    y += 4 + 26;
    y += heading(lang == LANG_PT ? "VER SOMENTE:" : "SHOW ONLY:",
                 24, y, 912, 2, C_SOFT, 0, 1, 1, 24) + 18;
    const int count = cat_section_count(c);
    /* Two uniform 344x48 tiles per row. The final odd tile (normally All
     * plus four sections = five buttons) is centered, not left-aligned. */
    const int columns = 2;
    const int total_buttons = count + 1;
    const float fw = 344.f, gap_x = 16.f, gap_y = 70.f;
    for (int slot = -1; slot < count; ++slot) {
        int number = 0;
        for (int i = 0; i < g_nposts; ++i)
            if (slot < 0 ? g_posts[i].cat == c : cat_post_in_section(c, slot, &g_posts[i]))
                ++number;
        char label[112];
        snprintf(label, sizeof(label), "%s (%d)",
                 slot < 0 ? (lang == LANG_PT ? "Todos" : "All") : cat_filter_label(c, slot),
                 number);
        int position = slot + 1, row = position / columns;
        int row_remaining = total_buttons - row * columns;
        int row_count = row_remaining < columns ? row_remaining : columns;
        float row_w = row_count * fw + (row_count - 1) * gap_x;
        float row_left = (SCR_W - row_w) / 2.f;
        float fx = row_left + (position % columns) * (fw + gap_x);
        float fy = y + row * gap_y;
        button(label, fx, fy, fw, cat_filter[c] == slot,
               A_CAT_FILTER, slot, NULL, view_t0 + .08 + .035 * position);
    }
    y += ((total_buttons + columns - 1) / columns) * gap_y + 12.f;
    rect(24, y, 912, 4, C_LINE);
    y += 32;
    for (int section = 0; section < count; ++section) {
        if (cat_filter[c] >= 0 && cat_filter[c] != section) continue;
        int ids[MAX_POSTS], n = 0;
        for (int i = 0; i < g_nposts; i++)
            if (cat_post_in_section(c, section, &g_posts[i])) ids[n++] = i;
        char gh[112];
        snprintf(gh, sizeof(gh), "%s (%d)", cat_section_label(c, section), n);
        y += heading(gh, 24, y, 912, 2, C_SOFT, 0, 1, 1, 24) + 28;
        if (n) y += grid(ids, n, y, view_t0 + .12 + section * .08, .06);
        else y += para(TXT(S_NOTHING), 24, y, 912, C_MUTED, 1, 1, 0);
        y += 44;
    }
    if (c == CAT_PORTS || c == CAT_TOOLS) {
        float w = title_width(keep_upper(TXT(S_REPOS)), 2) + 48;
        if (w > 912) w = 912;
        button(TXT(S_REPOS), SCR_W / 2 - w / 2, y, w, 0, A_URL, 0, "https://wolffsroom.github.io/WolffsStore/", -1);
        y += 48 + 24;
    }
    float bw = title_width(keep_upper(TXT(S_BACK_HOME)), 2) + 64;
    if (bw > 912) bw = 912;
    button(TXT(S_BACK_HOME), (SCR_W - bw) / 2, y, bw, 1, A_HOME, 0, NULL, -1);
    y += 48;
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

static int post_date_display(const char *iso, char *out, size_t len) {
    if (!iso || strlen(iso) != 10 || iso[4] != '-' || iso[7] != '-') return 0;
    for (int i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && (iso[i] < '0' || iso[i] > '9')) return 0;
    }
    int y, m, d;
    if (sscanf(iso, "%4d-%2d-%2d", &y, &m, &d) != 3 ||
        y < 1 || m < 1 || m > 12 || d < 1) return 0;
    static const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int last = days[m - 1] + (m == 2 && (y % 400 == 0 || (y % 4 == 0 && y % 100 != 0)));
    if (d > last) return 0;
    if (lang == LANG_PT) snprintf(out, len, "%02d/%02d/%04d", d, m, y);
    else {
        static const char *months[] = {
            "January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December"
        };
        snprintf(out, len, "%s %d, %d", months[m - 1], d, y);
    }
    return 1;
}

/* Journal/News: brief feed-style native article, media and timestamp.
 * Separate from verbose release/port details. */
static void build_news(int pi) {
    Post *p = &g_posts[pi];
    PostText *t = &p->t[lang];
    layout_begin();
    float y = page_bar(header() + 24, A_BACK, TXT(S_CAT3), t->title) + 24;
    rect(80, y, 800, 5, C_LINE);
    y += 22;
    y += heading(t->title, 90, y, 780, 3, C_WHITE, C_SHADOW, 1, 0, 32) + 12;
    char stamp[54];
    if (post_date_display(p->updated[0] ? p->updated : p->date, stamp, sizeof(stamp)))
        y += para(stamp, 90, y, 780, C_MUTED, 0, 2, 0) + 20;
    if (t->about && t->about[0])
        y += para(t->about, 90, y, 780, C_TEXT, 0, 36, 0) + 18;
    for (int j = 0; j < t->nblocks; j++) {
        Block *block = &t->blocks[j];
        if (block->type == BLK_SEC)
            y += heading(block->text, 90, y, 780, 2, C_SOFT, 0, 0, 1, 24) + 8;
        else if (block->type == BLK_ITEM) {
            rect(93, y + 9, 6, 6, C_GOLD);
            y += para(block->text, 112, y, 744, C_TEXT, 0, 16, 0) + 10;
        } else y += para(block->text, 90, y, 780, C_TEXT, 0, 20, 0) + 10;
    }
    if (cover_tex[pi]) {
        float h = 780 * 9 / 16.f;
        rect(90, y, 780, h, C_DARK);
        Op *pic = op(OP_TEX, 94, y + 4, 772, h - 8);
        pic->tex = cover_tex[pi];
        frame(90, y, 780, h, C_BORDER, C_WHITE, -1);
        y += h + 24;
    }
    if (shot_count) {
        const float sw=382, sh=214;
        for (int j=0; j<shot_count; j++) {
            const float x=90 + (j % 2) * 398;
            const float py=y + (j/2) * (sh+18);
            int it=item(x,py,sw,sh,A_SHOT,j,NULL);
            rect(x,py,sw,sh,C_DARK);
            Op *pic=op(OP_TEX,x+4,py+4,sw-8,sh-8);
            pic->tex=shot_tex[j];
            frame(x,py,sw,sh,C_BORDER,C_WHITE,it);
        }
        y += ((shot_count+1)/2) * (sh+18) + 4;
    }
    if (p->video_file || p->video) {
        y += heading(TXT(S_VIDEO),90,y,780,2,C_SOFT,0,0,1,24) + 18;
        int it = p->video_file
            ? item(90,y,780,250,A_VIDEO,pi,NULL)
            : item(90,y,780,250,A_URL,0,p->video);
        rect(90,y,780,250,C_DARK);
        if (cover_tex[pi]) {
            Op *o=op(OP_TEX,94,y+4,772,242);
            o->tex=cover_tex[pi];
        }
        frame(90,y,780,250,C_BORDER,C_WHITE,it);
        op(OP_PLAY,448,y+83,64,64);
        y += 270;
    }
    rect(80,y,800,5,C_LINE);
    page_h=footer(y+28);
}

static void build_poll(int pi) {
    Post *p=&g_posts[pi];
    PollState ps;
    poll_state(&ps);
    const int ended=poll_local_ended(p->poll_ends_at);
    layout_begin();
    float y=page_bar(header()+24,A_BACK,TXT(S_CAT3),p->t[lang].title)+28;
    char title_text[144];
    snprintf(title_text,sizeof(title_text),"%s",
        lang==LANG_PT ? "ENQUETE!" : "POLL!");
    y+=heading(title_text,24,y,912,4,C_WHITE,C_SHADOW,1,0,48)+24;
    y+=para(p->poll_question,24,y,912,C_WHITE,0,5,1)+24;
    char time_label[96];
    snprintf(time_label,sizeof(time_label),
             lang==LANG_PT ? "Resultado em %d dia(s)" : "Results in %d day(s)",
             poll_days_remaining(p->poll_ends_at));
    y+=para(time_label,24,y,912,C_MUTED,0,2,0)+16;
    /* The poll's own cover appears centered before its voting options. */
    if (cover_tex[pi]) {
        y += 18;
        rect(176,y,608,346,C_DARK);
        Op *cover = op(OP_TEX,180,y+4,600,338);
        cover->tex = cover_tex[pi];
        frame(176,y,608,346,C_BORDER,C_WHITE,-1);
        y += 346 + 28;
    }
    rect(24,y,912,4,C_LINE);
    y+=24;
    int matched=strcmp(ps.id,p->poll_id)==0;
    int answered=matched ? ps.answered : poll_local_answered(p->poll_id);
    if(ended) {
        const char *heading_text=lang==LANG_PT?"RESULTADOS":"RESULTS";
        y+=heading(heading_text,24,y,912,3,C_SOFT,0,0,1,32)+20;
        if(matched && ps.results_ready) {
            for(int i=0;i<p->poll_count;i++) {
                int pct=ps.total ? (ps.votes[i]*100+ps.total/2)/ps.total : 0;
                char label[172], right[24];
                snprintf(label,sizeof(label),"%s%s",p->poll_options[i],
                         ps.selected==i ? (lang==LANG_PT?" * SUA ESCOLHA":" * YOUR CHOICE") : "");
                snprintf(right,sizeof(right),"%d%%",pct);
                body(keep(label),24,y,ps.selected==i ? C_GOLD : C_WHITE);
                title(keep(right),844,y,2,C_GOLD,0);
                y+=32;
                Op *bar=op(OP_BAR,24,y,912,24);
                bar->scale=pct;
                bar->bold=3; /* animate result fill from 0% on opening */
                bar->pop=view_t0+.08+i*.15;
                y+=56;
            }
        } else {
            y+=para(!poll_api_ready()
                     ? (lang==LANG_PT ? "Resultados indisponíveis: servidor não configurado." :
                                         "Results unavailable: backend not configured.")
                     : (matched && ps.error)
                     ? (lang==LANG_PT ? "Falha ao obter resultados. Tente abrir novamente." :
                                         "Unable to load results. Reopen the poll to retry.")
                     : (lang==LANG_PT ? "Carregando resultados..." : "Loading results..."),
                     24,y,912,C_MUTED,0,4,0)+16;
        }
    } else if(answered) {
        y+=heading(lang==LANG_PT?"RESPONDIDA":"ANSWERED",
                   24,y,912,3,C_GOLD,0,0,1,36)+22;
        y+=para(lang==LANG_PT
                ? "Obrigado por participar! A resposta foi registrada."
                : "Thank you for participating! Your vote is recorded.",
                24,y,912,C_WHITE,0,3,0)+16;
        y+=para(time_label,24,y,912,C_SOFT,0,2,0)+24;
    } else {
        y+=para(lang==LANG_PT ? "Escolha uma opção:" : "Choose one option:",
                24,y,912,C_SOFT,0,2,0)+18;
        for(int i=0;i<p->poll_count;i++) {
            char label[178];
            /* Selection is represented by the filled pixel button itself. */
            snprintf(label,sizeof(label),"%s",p->poll_options[i]);
            int button_id=button(keep(label),24,y,912,poll_selected==i,
                                 A_POLL_CHOOSE,i,NULL,view_t0+.07+i*.09);
            y+=(button_id>=0?items[button_id].h:48)+16;
        }
        if(matched && ps.busy) {
            y+=para(lang==LANG_PT?"Enviando/consultando voto..." :
                    "Submitting/checking vote...",24,y,912,C_MUTED,0,3,0)+22;
        } else if(matched && ps.error) {
            y+=para(lang==LANG_PT?
                    "Falha no serviço. Tente novamente com HTTPS ativo.":
                    "Voting service unavailable. Please retry with secure HTTPS.",
                    24,y,912,C_MUTED,0,3,0)+18;
        }
        y += 24;
        if (poll_selected >= 0 && poll_api_ready() && !(matched && ps.busy)) {
            int button_id=button(lang==LANG_PT?"CONFIRMAR VOTO":"CONFIRM VOTE",
                                  24,y,912,1,A_POLL_SEND,0,NULL,view_t0+.45);
            y+=(button_id>=0?items[button_id].h:48)+16;
        } else if(!poll_api_ready()) {
            y+=para(lang==LANG_PT?
                    "Votação temporariamente indisponível: servidor não configurado.":
                    "Voting unavailable: backend not configured.",
                    24,y,912,C_MUTED,0,4,0)+16;
        } else if (matched && ps.busy) {
            y += 12;
        } else {
            y+=para(lang==LANG_PT?"Selecione uma opção para confirmar o voto.":
                     "Select an option to confirm your vote.",
                     24,y,912,C_MUTED,1,2,0)+12;
        }
        /* Permanent ballot box immediately below the confirmation area. */
        poll_box_page_y = y + 12;
        if (poll_box_tex && (poll_anim_post != pi ||
            now_s - poll_anim_t0 >= 1.65 || now_s < poll_anim_t0)) {
            Op *box=op(OP_TEX,376,poll_box_page_y,208,163);
            box->tex=poll_box_tex;
            box->pop=view_t0+.45;
        }
        y = poll_box_page_y + 163 + 14;
    }
    y+=32;
    rect(24,y,912,4,C_LINE);
    y+=20;
    y+=para(lang==LANG_PT?
            "Privacidade: somente um identificador derivado por hash sai do Vita.":
            "Privacy: only a hashed device pseudonym leaves your Vita.",
            24,y,912,C_MUTED,0,3,0)+12;
    page_h=footer(y);
}

static void build_post(int pi) {
    if(pi>=0 && pi<g_nposts && g_posts[pi].poll_id[0]) {
        build_poll(pi);
        return;
    }
    if(pi>=0 && pi<g_nposts && g_posts[pi].cat==CAT_REVIEWS &&
       g_posts[pi].subcat==SUB_NEWS) {
        build_news(pi);
        return;
    }
    layout_begin();
    Post *p = &g_posts[pi];
    PostText *t = &p->t[lang];
    float y = page_bar(header() + 24, A_BACK, TXT(S_CAT0 + p->cat), t->title) + 32;
    y += heading(t->title, 24, y, 912, 4, C_WHITE, C_SHADOW, 1, 0, 48) + 8;
    char line[512];
    snprintf(line, sizeof(line), "%s \xb7 %s", t->tag, t->status);
    upper_latin1(line);
    y += para(line, 24, y, 912, C_MUTED, 1, 2, 0) + 12;
    char published[40], updated[40];
    if (post_date_display(p->date, published, sizeof(published))) {
        snprintf(line, sizeof(line), "%s: %s", TXT(S_POST_PUBLISHED), published);
        if (post_date_display(p->updated, updated, sizeof(updated))) {
            size_t used = strlen(line);
            snprintf(line + used, sizeof(line) - used, "  \xb7  %s: %s", TXT(S_POST_UPDATED), updated);
        }
        y += para(line, 24, y, 912, C_MUTED, 1, 3, 0) + 16;
    } else y += 4;
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
                              : d->type == DL_TAI_PLUGIN
                              ? (st.installed ? TXT(S_DL_REINSTALL) : TXT(S_DL_INSTALL))
                              : d->type == DL_TRANS || d->type == DL_APP_FILES
                              ? (st.installed ? TXT(S_DL_TRANS_RE) : TXT(S_DL_TRANS))
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
            } else if (d->type == DL_APP_FILES) {
                char to[128];
                snprintf(to, sizeof(to), "ux0:app/%s/", d->ntids ? d->tids[0] : "?");
                yy += para(to, rx + in, yy, iw, C_MUTED, 0, 2, 0) + 8;
                op(OP_WARN, rx + in, yy + 2, 20, 20);
                yy += para(TXT(S_APP_LIVEAREA), rx + in + 28, yy, iw - 28, C_TEXT, 0, 5, 0) + 10;
            } else if (d->type == DL_DATA) {
                yy += para(TXT(S_DATA_TO), rx + in, yy, iw, C_MUTED, 0, 3, 0) + 10;
            } else if (d->type == DL_TAI_PLUGIN) {
                const char *notice=lang==LANG_PT
                    ? "Backup do taiHEN; instala em ur0:tai. Reinicie o jogo."
                    : "Backs up taiHEN; installs to ur0:tai. Restart the game.";
                yy += para(notice, rx + in, yy, iw, C_TEXT, 0, 4, 0) + 10;
            } else if (d->type == DL_PLUGIN || d->type == DL_FILE) {
                const char *notice=d->type==DL_PLUGIN
                    ? (lang==LANG_PT ? "Arquivo em ux0:Downloads/Plugins." : "Saved to ux0:Downloads/Plugins.")
                    : TXT(S_PLUGIN_NOTE);
                yy += para(notice, rx + in, yy, iw, C_TEXT, 0, 4, 0) + 10;
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
    /* Download links get the same priority as native install buttons:
     * always the first side box, before Details and progress metrics. */
    if (!p->ndls && t->nlinks) {
        int bg = box_begin(rx, ry, rw);
        float yy = ry + in;
        yy += heading(TXT(S_DOWNLOADS), rx + in, yy, rw - 2 * in, 2, C_WHITE, 0, 0, 1, 24) + 16;
        for (int i = 0; i < t->nlinks; i++) {
            int it = button(t->links[i].a, rx + in, yy, rw - 2 * in, i == 0, A_URL, 0, t->links[i].b, -1);
            yy += (it >= 0 ? items[it].h : 48) + 12;
        }
        ry += box_end(bg, rx, ry, rw, yy - 12) + 36;
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
        int n = b ? t->nsrcs : 0; /* manual Downloads was shown above Details */
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
    case J_RESOLVE: return TXT(S_RESOLVING_LINK);
    case J_DOWNLOAD: return j->pr.done > 0 ? TXT(S_DOWNLOADING) : TXT(S_CONNECTING);
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
        const char *display_name = g_posts[pi].t[lang].title;
        if (!display_name || !display_name[0])
            display_name = e->name[0] ? e->name : d->name;
        if (wrap_title(display_name, 2, 560, lines, 1, buf2, sizeof(buf2)))
            title(keep(lines[0]), 48, y + 20, 2, C_WHITE, 0);
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

static unsigned progress_color(int pct) {
    return pct < 40 ? C_PROGRESS_LOW : pct < 80 ? C_PROGRESS_MID : C_PROGRESS_HIGH;
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
        vita2d_draw_rectangle(x + 4 + sx, y + 4, sw, 16, progress_color(pct));
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
    int busy = j->state == J_EXTRACT || j->state == J_INSTALL ||
               j->state == J_QUEUED || j->state == J_RESOLVE ||
               (j->state == J_DOWNLOAD && j->pr.done <= 0);
    if (busy && (j->state != J_EXTRACT || j->phase)) {
        /* instalando/na fila: listras andando */
        int off = (int)(now_s * 35) % 24;
        for (float sx = -24 + off; sx < w - 16; sx += 24)
            if (sx >= 0) vita2d_draw_rectangle(x + 8 + sx, y + h - 12, 12, 4, col_alpha(C_SOFT, al));
    } else {
        seg_bar(x + 8, y + h - 12, w - 16, 4, k, col_alpha(C_WHITE, al), col_alpha(RGBA8(0x24, 0x24, 0x24, 0xFF), al));
    }
    char t[64];
    if ((j->state == J_DOWNLOAD && j->pr.done > 0) ||
        (j->state == J_EXTRACT && !j->phase))
        snprintf(t, sizeof(t), "%s %d%%", job_state_text(j), (int)(k * 100));
    else snprintf(t, sizeof(t), "%s", job_state_text(j));
    upper_latin1(t);
    title_draw(t, x + (w - title_width(t, 2)) / 2, y + 12, 2, col_alpha(C_WHITE, al));
}

/* linha da tela de downloads: estado, barra, velocidade, mensagem */
static void draw_jobrow(const Op *o, float x, float y, float al) {
    Job *j = job_by_id(o->scale);
    if (!j) return;
    float k = job_frac(j);
    /* Warm scanning light during multi-stage download/extract/install. */
    if (j->state < J_DONE) {
        float scan = fmodf((float)now_s * 105.f, o->w + 64.f) - 32.f;
        float left = fmaxf(0.f, scan);
        float sw = fmaxf(0.f, fminf(32.f, o->w - left));
        vita2d_draw_rectangle(x, y + 26, o->w, 3, col_alpha(C_LINE, al));
        if (sw > 0) vita2d_draw_rectangle(x + left, y + 26, sw, 3,
                                           col_alpha(C_GOLD, al * .7f));
    }
    char t[256], a[24], b[24];
    fmt_size(j->pr.done, a, sizeof(a));
    fmt_size(j->pr.total, b, sizeof(b));
    if (j->state == J_DOWNLOAD && j->pr.done > 0) {
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
        int busy = (j->state == J_EXTRACT && j->phase) || j->state == J_INSTALL ||
                   j->state == J_RESOLVE || (j->state == J_DOWNLOAD && j->pr.done <= 0);
        vita2d_draw_rectangle(x, y + 26, o->w, 16, col_alpha(C_DARK, al));
        if (busy) {
            int off = (int)(now_s * 35) % 24;
            for (float sx = -24 + off; sx < o->w; sx += 24)
                if (sx >= 0) vita2d_draw_rectangle(x + sx, y + 30, 12, 8, col_alpha(C_SOFT, al));
        } else {
            seg_bar(x, y + 30, o->w, 8, k, col_alpha(C_WHITE, al), col_alpha(RGBA8(0x24, 0x24, 0x24, 0xFF), al));
        }
    }
}

/* Native five-card stack. No browser, no duplicate bitmaps and no glow on
 * the side cards: the center draws last and remains visually in front. */
static void draw_deck_card(const Op *o, float x, float y, float alpha, int f) {
    if (o->scale < 0 || o->scale >= g_nposts) return;
    int slot = o->bold - 2;
    const float shade = slot == 0 ? 1.f : (abs(slot) == 1 ? .78f : .36f);
    float a = alpha * shade;
    /* Short horizontal slide after a D-pad or touch selection. */
    float moving = (float)(now_s - deck_moved_t0);
    if (moving >= 0.f && moving < .24f)
        x += deck_direction * 20.f * (1.f - moving / .24f);
    if (f) y -= slot == 0 ? 10.f : (abs(slot) == 1 ? 8.f : 5.f);
    Post *p = &g_posts[o->scale];
    const PostText *t = &p->t[lang];
    float w = o->w, h = o->h;
    unsigned frame_color = slot == 0 ? C_WHITE :
                           abs(slot) == 1 ? C_BTNBRD : C_BORDER;
    /* Slight shadow only, no focus halo or scaling for side cards. */
    vita2d_draw_rectangle(x + 6, y + 8, w, h, col_alpha(C_SH2, alpha * .68f));
    vita2d_draw_rectangle(x, y, w, h, col_alpha(C_PANEL, a));
    float ch = (w - 8) * 9.f / 16.f;
    if (cover_tex[o->scale]) {
        tex_draw(cover_tex[o->scale], x + 4, y + 4, w - 8, ch,
                 col_alpha(C_WHITE, a));
    } else {
        vita2d_draw_rectangle(x + 4, y + 4, w - 8, ch, col_alpha(C_DARK, a));
    }
    vita2d_draw_rectangle(x + 4, y + 4 + ch, w - 8, 4,
                           col_alpha(C_BTNBRD, a));
    frame_rect(x, y, w, h, 4, col_alpha(frame_color, a));
    float ty = y + 12 + ch, tw = w - 24;
    char *lines[4];
    char buffer[480];
    int tag_lines = wrap_body(t->tag, (int)tw, lines, 1, buffer, sizeof(buffer));
    if (tag_lines) body_draw(lines[0], x + 12, ty, col_alpha(C_MUTED, a));
    ty += 22;
    int count = wrap_title(t->title, 2, (int)tw, lines, 2, buffer, sizeof(buffer));
    for (int i = 0; i < count; i++)
        title_draw(lines[i], x + 12, ty + i * 24, 2, col_alpha(C_WHITE, a));
    /* Status at the bottom, clipped to a single readable line. */
    count = wrap_body(t->status, (int)tw, lines, 1, buffer, sizeof(buffer));
    if (count && y + h - 28 > ty + 10)
        body_draw(lines[0], x + 12, y + h - 26, col_alpha(C_SOFT, a));
    if (slot == 0 && f) {
        /* Protagonist only; sides only lift by 5–8 px. */
        frame_rect(x - 3, y - 3, w + 6, h + 6, 3, col_alpha(C_WHITE, alpha));
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
        if (o->type == OP_TITLE && o->bold == 3 && !f) {
            /* Low-frequency, subtle alpha pulse; no whole-screen flashing. */
            float pulse = .72f + .28f * (.5f + .5f * sinf((float)now_s * 2.1f));
            c = col_alpha(C_GOLD, al * pulse);
        }
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
        case OP_BAR: {
            int pct=o->scale;
            if(o->bold==3) {
                float elapsed=fmaxf(0.f,fminf(1.f,(float)((now_s-o->pop)/.70)));
                elapsed=1.f-powf(1.f-elapsed,3.f);
                pct=(int)(pct*elapsed+.5f);
            }
            draw_bar(x,y,o->w,pct);
            break;
        }
        case OP_STAGE: draw_stage(o, oy); break;
        case OP_DECK_CARD: draw_deck_card(o, x, y, al, f); break;
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
        case OP_TRI: /* back arrow or mirrored right arrow in carousel */
            for (int i = 0; i < 4; i++) {
                int dx = o->scale == 1 ? (3 - i) * 4 : i * 4;
                vita2d_draw_rectangle(x + dx, y + 6 - i * 2, 4, 4 + i * 4, c);
            }
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

/* Native vote feedback overlay. Always in screen coordinates (not page
 * coordinates): users can submit at any scroll position and still see it.
 * This is a submission animation, not a success acknowledgement. */
static void draw_poll_vote_animation(void) {
    if (view != V_POST || view_post != poll_anim_post ||
        !poll_box_tex || !poll_paper_tex || poll_box_page_y < 0) return;
    float elapsed = (float)(now_s - poll_anim_t0);
    if (elapsed < 0.f || elapsed >= 1.65f) return;
    float fade = fminf(1.f, fmaxf(0.f, (1.65f - elapsed) / .30f));
    /* Follow the page's ballot box, directly beneath the confirm button.
     * The controller scrolls there on submission, even for long polls. */
    float top = poll_box_page_y - cam;
    float t = fminf(1.f, fmaxf(0.f, (elapsed - .12f) / .78f));
    t = t * t * (3.f - 2.f * t);
    float paper_alpha = fade;
    if (elapsed > .90f)
        paper_alpha *= fmaxf(0.f, 1.f - (elapsed - .90f) / .16f);
    /* Paper first, ballot box second: slot obscures the inserted part. */
    tex_draw(poll_paper_tex, 427, top - 126 + 120 * t, 106, 121,
             col_alpha(C_WHITE, paper_alpha));
    tex_draw(poll_box_tex, 376, top, 208, 163,
             col_alpha(C_WHITE, fade));
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
    if (v == V_CAT) {
        if (arg >= 0 && arg < CAT_COUNT &&
            (from_view == V_HOME || (from_view == V_CAT && view_cat != arg)))
            cat_filter[arg] = -1;
        view_cat = arg;
    }
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
        focus = find_item(A_CAT_FILTER, cat_filter[view_cat]);
        if (focus < 0) focus = find_item(A_CAT_FILTER, -1);
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

static void close_update_notes(void) {
    update_notes_visible = 0;
    fs_write(WS_DATA "/release_notes_seen.txt", WS_VERSION,
             (int)strlen(WS_VERSION));
    sfx("confirm");
}

static void close_plugin_advice(int install) {
    if (plugin_advice_post < 0) return;
    int pi = plugin_advice_post, di = plugin_advice_dl;
    plugin_advice_post = plugin_advice_dl = -1;
    if (install) {
        Post *p = &g_posts[pi];
        const char *project = p->t[lang].title;
        if (jobs_add(p, di, project && project[0] ? project : p->dls[di].name, 0) >= 0)
            sfx("download");
    } else sfx("cancel");
}

static void close_translation_confirm(int install) {
    if (trans_confirm_post < 0) return;
    int pi = trans_confirm_post, di = trans_confirm_dl;
    if (install && (trans_confirm_backup || trans_confirm_base_missing ||
                    (!trans_confirm_app && !trans_confirm_plugin))) {
        sfx("back");
        return; /* backup existente ou jogo base ausente: não instalar */
    }
    trans_confirm_post = trans_confirm_dl = -1;
    if (install) {
        Post *p = &g_posts[pi];
        if (jobs_add(p, di, p->dls[di].name, 1) >= 0) {
            sfx("confirm");
            if (p->dls[di].type == DL_APP_FILES)
                toast(TXT(S_APP_LIVEAREA));
            else
                toast(TXT(S_TRANS_INSTALLING));
        }
    } else sfx("back");
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
    float progress = fmaxf(0.f, fminf(1.f, (float)(e / .23)));
    float k = 1.f - powf(1.f - progress, 3.f);
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
        float stagger = fmaxf(0.f, fminf(1.f, (float)((e - r * .038) / .20)));
        stagger = 1.f - powf(1.f - stagger, 3.f);
        float ry = y + 90 + r * rh + 10.f * (1.f - stagger);
        float rx = x + 24 + 16.f * (1.f - stagger), rw = w - 48;
        int focus_row = r == settings_row;
        if (focus_row) {
            float glow = .85f + .15f * sinf((float)now_s * 4.f);
            vita2d_draw_rectangle(rx, ry, rw, rh - 7, col_alpha(C_WHITE, k * glow));
        }
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
                /* Installed records can retain old mirror labels; the
                 * project title is the canonical download queue name. */
                const char *project = g_posts[i].t[lang].title;
                if (g_posts[i].dls[e->dl].type == DL_TRANS ||
                    g_posts[i].dls[e->dl].type == DL_APP_FILES) {
                    open_translation_confirm(i, e->dl);
                } else if (g_posts[i].dls[e->dl].type == DL_TAI_PLUGIN) {
                    open_plugin_advice(i, e->dl);
                } else if (g_posts[i].dls[e->dl].type == DL_VPK &&
                           g_posts[i].dls[e->dl].advisory_plugin &&
                           g_posts[i].dls[e->dl].advisory_plugin[0]) {
                    open_plugin_advice(i, e->dl);
                } else if (jobs_add(&g_posts[i], e->dl,
                             project && project[0] ? project : g_posts[i].dls[e->dl].name, 0) >= 0)
                    sfx("confirm");
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
        /* Do not label the queue by a mirror/CDN asset, e.g. "Download
         * alternativo (Google Drive)". Keep the actual post name visible. */
        if (p->dls[x->arg].type == DL_TRANS ||
            p->dls[x->arg].type == DL_APP_FILES) {
            open_translation_confirm(view_post, x->arg);
            break;
        }
        if(p->dls[x->arg].type == DL_TAI_PLUGIN) {
            open_plugin_advice(view_post,x->arg);
            break;
        }
        if (p->dls[x->arg].type == DL_VPK &&
            p->dls[x->arg].advisory_plugin &&
            p->dls[x->arg].advisory_plugin[0]) {
            open_plugin_advice(view_post, x->arg);
            break;
        }
        const char *project = p->t[lang].title;
        if (jobs_add(p, x->arg,
                     project && project[0] ? project : p->dls[x->arg].name, 0) >= 0)
            sfx("download");
        break;
    }
    case A_CANCEL:
        jobs_cancel(x->arg);
        sfx("cancel");
        break;
    case A_CLEAR:
        jobs_clear_finished();
        sfx("tick");
        rebuild();
        focus = find_item(A_BACK, 0);
        break;
    case A_POST:
        sfx("open");
        if(x->arg>=0 && x->arg<g_nposts && g_posts[x->arg].poll_id[0]) {
            Post *poll=&g_posts[x->arg];
            poll_selected=-1;
            poll_box_page_y=-1.f;
            poll_anim_post=-1;
            poll_open(poll->poll_id,poll->poll_ends_at,
                      poll->poll_count,poll_local_ended(poll->poll_ends_at));
        }
        go(V_POST, x->arg);
        break;
    case A_POLL_CHOOSE:
        if(view==V_POST && view_post>=0 && g_posts[view_post].poll_id[0]) {
            poll_selected=x->arg;
            sfx("tick");
            rebuild();
        }
        break;
    case A_POLL_SEND:
        if(view==V_POST && view_post>=0 && poll_selected>=0 &&
           g_posts[view_post].poll_id[0] && poll_vote(poll_selected)) {
            poll_anim_post = view_post;
            poll_anim_t0 = now_s;
            /* Bring the box into view during confirmation. */
            if (poll_box_page_y >= 0) {
                float desired = poll_box_page_y + 190.f - (SCR_H - hint_h);
                if (desired > cam_target) cam_target = desired;
                cam_clamp();
            }
            sfx("download");
            rebuild();
        } else sfx("notice");
        break;
    case A_DECK_CARD: {
        if (view != V_HOME || now_s < deck_access_t0 ||
            deck_n == 0 || x->arg < 0 || x->arg >= deck_n) break;
        int selected = x->arg;
        if (selected == deck_pos) {
            sfx("open");
            go(V_POST, deck_order[selected]);
        } else {
            int step = selected - deck_pos;
            if (step > deck_n / 2) step -= deck_n;
            if (step < -deck_n / 2) step += deck_n;
            deck_direction = step < 0 ? -1 : 1;
            deck_pos = selected;
            deck_moved_t0 = now_s;
            sfx("tick");
            rebuild();
            focus = find_item(A_DECK_CARD, deck_pos);
            cam_follow();
            cam_clamp();
        }
        break;
    }
    case A_DECK_ARROW: {
        if (view != V_HOME || now_s < deck_access_t0 || deck_n < 2) break;
        int direction = x->arg < 0 ? -1 : 1;
        deck_direction = direction;
        deck_pos = (deck_pos + direction + deck_n) % deck_n;
        deck_moved_t0 = now_s;
        sfx("tick");
        rebuild();
        focus = find_item(A_DECK_CARD, deck_pos);
        cam_follow();
        cam_clamp();
        break;
    }
    case A_PEEK:
        if (x->arg == CAT_REVIEWS) journal_mark_seen();
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
        if (x->arg == CAT_REVIEWS) journal_mark_seen();
        sfx("open");
        go(V_CAT, x->arg);
        break;
    case A_CAT_FILTER: {
        if (view != V_CAT || view_cat < 0 || view_cat >= CAT_COUNT) break;
        int selected = x->arg;
        if (selected < -1 || selected >= cat_section_count(view_cat)) break;
        cat_filter[view_cat] = selected;
        sfx("tick");
        rebuild();
        focus = find_item(A_CAT_FILTER, selected);
        if (focus < 0) focus = find_item(A_CAT_FILTER, -1);
        cam_follow();
        cam_clamp();
        break;
    }
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
    if (view == V_HOME && now_s < deck_access_t0 &&
        focus >= 0 && focus < nitems &&
        (items[focus].act == A_DECK_CARD || items[focus].act == A_DECK_ARROW))
        return; /* Cards are not interactive while they are invisible. */
    if (view == V_HOME && now_s >= deck_access_t0 && focus >= 0 && focus < nitems &&
        (items[focus].act == A_DECK_CARD || items[focus].act == A_DECK_ARROW) &&
        (dir == 2 || dir == 3) && deck_n >= 2) {
        deck_direction = dir == 2 ? -1 : 1;
        deck_pos = (deck_pos + deck_direction + deck_n) % deck_n;
        deck_moved_t0 = now_s;
        sfx("tick");
        rebuild();
        focus = find_item(A_DECK_CARD, deck_pos);
        cam_follow();
        cam_clamp();
        return;
    }
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
    /* Initial catalog is synchronized BEFORE the title-screen question or
     * intro is allowed to begin. Cached content is the offline fallback,
     * not a reason to delay updating until after Home. */
    const int sc = 5, step = 8 * sc;
    /* Sync shows the animated logo without the paw icon. */
    float lw = title_width(LOGO, sc);
    for (int i = 0; LOGO[i]; i++) {
        char ch[2] = {LOGO[i], 0};
        double e = now_s - letter_hop[i];
        float hop = e >= 0 && e < .16 ? -10 : 0;
        float x = (SCR_W - lw) / 2 + i * step;
        title_draw(ch, x + 5, SCR_H / 2 - 38 + 5 + hop, sc, C_SHADOW);
        title_draw(ch, x, SCR_H / 2 - 38 + hop, sc, C_WHITE);
    }
    double elapsed = now_s - first_sync_t0;
    /* No loading copy is shown on quick synchronization. This quiet
     * preface is just the normal paw/logo, not a fake progress screen. */
    if (elapsed < 1.10) return;

    static const char *headings[2] = {
        "Updating the Store...", "Atualizando a loja..."
    };
    static const char *subtitles[2][5] = {
        {"Dusting off the ports", "Recompiling projects",
         "Fixing a bug or two", "Organizing the shelves",
         "Moving things around"},
        {"Tirando a poeira dos ports", "Recompilando projetos",
         "Corrigindo alguns bugs aqui e ali", "Organizando as prateleiras",
         "Movendo as coisas de lugar"}
    };
    const char *heading = headings[lang == LANG_PT ? 1 : 0];
    const int scale = 2, letters = (int)strlen(heading);
    float x = (SCR_W - title_width(heading, scale)) / 2;
    float y = SCR_H / 2 + 54;
    for (int i = 0; i < letters; i++) {
        char ch[2] = {heading[i], 0};
        int bounce = (int)(fabsf(sinf((float)elapsed * 5.2f - i * .36f)) * 8.f);
        float cx = x + i * (8 * scale);
        title_draw(ch, cx + 2, y + 2 - bounce, scale, C_SHADOW);
        title_draw(ch, cx, y - bounce, scale, C_WHITE);
    }
    if (elapsed >= 3.4) {
        /* Small rotating phrases only during a genuinely slow fetch. */
        int idx = (int)((elapsed - 3.4) / 3.0) % 5;
        const char *sub = subtitles[lang == LANG_PT ? 1 : 0][idx];
        body_draw(sub, (SCR_W - body_width(sub)) / 2,
                  SCR_H / 2 + 110, C_SOFT);
    }
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

static int updater_cleanup_owned(const char *id);

static void launch_updater(void) {
    /* Prepare the updater from the downloaded VPK; keep its registered
     * LiveArea title installed to avoid a forced SceShell transition. */
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
    /* Deleting an installed title while Store is foreground makes SceShell
     * refresh the LiveArea/app database; on real PS Vita this has been
     * observed to eject Store to LiveArea even when DeletePkg succeeds.
     * Re-promote the owned WFFS00043 package as an in-place update instead.
     * The harmless helper bubble remains installed for the next upgrade. */
    if (ok && fs_exists("ux0:app/WFFS00043/sce_sys/param.sfo") &&
        !updater_cleanup_owned("WFFS00043")) {
        ws_log("updater: installed WFFS00043 is not our helper; refuse overwrite");
        toast("Updater existente nao verificado. Nenhum app foi removido.");
        upd_job = -1;
        upd_skipped = 1;
        return;
    }
    if (ok) {
        ws_log("updater: promoting reusable WFFS00043 in place (no DeletePkg)");
        ok = inst_promote(WS_PKG_UPD) == 0;
    }
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
    feat = featured_post_for_device();
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

/* Before reusing an existing updater bubble, verify both its TITLE_ID
 * and TITLE; never overwrite an unrelated title during in-place promotion. */
static int updater_cleanup_owned(const char *id) {
    char sfo[140], installed_id[32] = "", installed_title[100] = "";
    snprintf(sfo, sizeof(sfo), "ux0:app/%s/sce_sys/param.sfo", id);
    return fs_exists(sfo) &&
           sfo_get(sfo, "TITLE_ID", installed_id, sizeof(installed_id)) == 0 &&
           strcmp(installed_id, id) == 0 &&
           sfo_get(sfo, "TITLE", installed_title, sizeof(installed_title)) == 0 &&
           strcmp(installed_title, "Wolff's Store Updater") == 0;
}

/* Older updater binaries (including v1.0.10) leave this marker behind.
 * A package delete during any Store session - including its initialization -
 * can force SceShell to drop Store back into LiveArea. Clearing ONLY the
 * obsolete marker is safe; keep WFFS00043 installed for reuse. */
static void updater_retire_marker_at_startup(void) {
    const char *marker = WS_DATA "/updater_cleanup.pending";
    if (!fs_exists(marker)) return;
    ws_log("updater: old cleanup marker found; preserving helper to avoid LiveArea exit");
    fs_rm_rf(marker);
    if (fs_exists(marker))
        ws_log("updater: could not clear old marker; skipping destructive cleanup");
}

static void tick_store(void) {
    PollState poll_snapshot;
    poll_state(&poll_snapshot);
    if(poll_snapshot.version != poll_seen_version) {
        poll_seen_version = poll_snapshot.version;
        if(poll_snapshot.answered && feat>=0 && feat<g_nposts &&
           g_posts[feat].poll_id[0]) feat=featured_post_for_device();
        if(view==V_POST && view_post>=0 && view_post<g_nposts &&
           g_posts[view_post].poll_id[0] &&
           !strcmp(g_posts[view_post].poll_id,poll_snapshot.id)) rebuild();
    }
    int intro_done = started && now_s - intro_t0 >= T_DONE;
    int st = sync_state();
    if (first_sync && st == SYNC_READY) {
        /* apply_sync() may synchronously decode many covers and rebuild
         * the featured mosaic. The 'now_s' value from the start of this
         * frame becomes stale during that work. If it is reused as the
         * typewriter origin, the entire question may render instantly.
         * Sample the monotonic clock AFTER preparing the featured post. */
        apply_sync();
        ws_log("intro: fresh catalog applied; starting full typewriter");
        now_s = clock_s() - app_clock_t0;
        first_sync = 0;
        title_t0 = now_s;
        title_cues();
        return;
    }
    if (first_sync && (st == SYNC_OFFLINE || st == SYNC_FAIL)) {
        /* The same timeline restart applies to offline cached catalogs. */
        ws_log("intro: offline catalog ready; starting full typewriter");
        now_s = clock_s() - app_clock_t0;
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
    if (update_notes_pending && !first_sync && intro_done &&
        !update_notes_visible && upd_job < 0 && view == V_HOME &&
        !settings_open && !vid_open && !snd_menu) {
        update_notes_pending = 0;
        update_notes_visible = 1;
        update_notes_t0 = now_s;
        sfx("updates");
    }
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
                int kind = j->kind == JK_TRANS ? DL_TRANS : j->kind == JK_APP_FILES ? DL_APP_FILES
                         : j->kind == JK_TAI_PLUGIN ? DL_TAI_PLUGIN
                         : j->kind == JK_PLUGIN ? DL_PLUGIN : j->kind == JK_FILE ? DL_FILE
                         : j->kind == JK_DATA ? DL_DATA : DL_VPK;
                char path[192];
                if (kind == DL_TRANS) snprintf(path, sizeof(path), "ux0:rePatch/%s", j->tids[0]);
                else snprintf(path, sizeof(path), "%s", j->msg);
                reg_set(j->slug, j->dl, kind, j->version, j->name, path);
                sfx("complete");
                if (j->kind == JK_TAI_PLUGIN)
                    snprintf(t,sizeof(t),"%s: %s",j->name,
                      lang==LANG_PT ? "instalado, reinicie o Vita" : "installed, reboot the Vita");
                else if (j->kind == JK_VPK) snprintf(t, sizeof(t), "%s %s", j->name, TXT(S_T_INSTALLED));
                else if (j->kind == JK_TRANS) snprintf(t, sizeof(t), "%s: %s (%s)", j->name, TXT(S_T_TRANS), j->msg);
                else if (j->kind == JK_DATA) snprintf(t, sizeof(t), "%s: %s", j->name, TXT(S_DATA_TO));
                else if (j->kind == JK_APP_FILES)
                    snprintf(t, sizeof(t), "%s. %s", TXT(S_T_TRANS), TXT(S_APP_LIVEAREA));
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
        /* Status punctuation follows the label; never prefix with !. */
        size_t n = strlen(t->text);
        snprintf(msg, sizeof(msg), n && (t->text[n - 1] == '!' ||
                                          t->text[n - 1] == '.' || t->text[n - 1] == '?')
                 ? "%s" : "%s!", t->text);
    } else {
        Job *j = current_job();
        if (!j) return;
        float k = job_frac(j);
        if (j->state == J_DOWNLOAD && j->pr.done > 0) {
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

/* Same square 4px border, 48-56px height, Press Start text and primary
 * white-fill visual language as the Store's normal Download buttons. */
static void draw_translation_action(float x, float y, float w, const char *label,
                                    int primary, int disabled) {
    float h = 48;
    unsigned fill = disabled ? C_LINE : primary ? C_WHITE : C_DARK;
    unsigned border = disabled ? C_BTNBRD : primary ? C_WHITE : C_BTNBRD;
    unsigned ink = disabled ? C_MUTED : primary ? C_DARK : C_WHITE;
    if (primary && !disabled) vita2d_draw_rectangle(x + 4, y + 4, w, h, C_SH1);
    vita2d_draw_rectangle(x, y, w, h, fill);
    frame_rect(x, y, w, h, 4, border);
    float tw = title_width(label, 2);
    title_draw(label, x + (w - tw) / 2, y + 16, 2, ink);
}

static void draw_translation_confirm(void) {
    if (trans_confirm_post < 0) return;
    const float x = 100, y = 64, w = 760, h = 410;
    vita2d_draw_rectangle(0, 0, SCR_W, SCR_H, RGBA8(0, 0, 0, 0xE5));
    halo_draw(x - 16, y - 16, w + 32, h + 32, 0, 1);
    vita2d_draw_rectangle(x, y, w, h, C_PANEL);
    frame_rect(x, y, w, h, 4, C_WHITE);
    const char *heading = TXT(S_TRANS_DIALOG_TITLE);
    title_draw(heading, x + (w - title_width(heading, 2)) / 2, y + 26, 2, C_WHITE);
    vita2d_draw_rectangle(x + 28, y + 76, w - 56, 4, C_LINE);

    if (trans_confirm_app) {
        char destination[128];
        snprintf(destination, sizeof(destination), "%s",
                 lang == LANG_PT ? "Destino: ux0:app/PCSF00007/"
                                 : "Destination: ux0:app/PCSF00007/");
        body_draw(destination, x + 28, y + 108, C_WHITE);
        body_draw(lang == LANG_PT ? "Backup: ux0:data/WolffsStore/backups/"
                                 : "Backup: ux0:data/WolffsStore/backups/",
                  x + 28, y + 156, C_MUTED);
        body_draw(TXT(S_APP_LIVEAREA), x + 28, y + 204, C_GOLD);
        const char *status = trans_confirm_base_missing
            ? (lang == LANG_PT ? "BLOQUEADO: instale o jogo PCSF00007 primeiro."
                               : "BLOCKED: install game PCSF00007 first.")
            : (lang == LANG_PT ? "RAR contendo apenas torrent sera rejeitado."
                               : "Torrent-only archives will be rejected.");
        body_draw(status, x + 28, y + 254,
                  trans_confirm_base_missing ? C_PROGRESS_LOW : C_MUTED);
    } else {
        body_draw(TXT(S_TRANS_REQUIRE), x + 28, y + 108, C_TEXT);
        /* Both status strings came through the standard UTF-8 -> Latin-1
         * UI pipeline. The detected status is highlighted in gold. */
        body_draw(trans_confirm_plugin ? TXT(S_TRANS_DETECTED) : TXT(S_TRANS_MISSING),
                  x + 28, y + 151, trans_confirm_plugin ? C_GOLD : C_PROGRESS_LOW);
        char note_utf8[180], note[180];
        if (trans_confirm_mod) {
            const Download *d = &g_posts[trans_confirm_post].dls[trans_confirm_dl];
            snprintf(note_utf8, sizeof(note_utf8),
                     lang == LANG_PT ? "Mod existente será salvo em %s_bkp."
                                     : "Existing mod backed up as %s_bkp.",
                     d->ntids ? d->tids[0] : "PCSE...");
        } else {
            snprintf(note_utf8, sizeof(note_utf8), "%s",
                     lang == LANG_PT ? "Nenhum mod existente será alterado."
                                     : "No existing mods will be changed.");
        }
        utf8_to_latin1(note_utf8, note, sizeof(note));
        body_draw(note, x + 28, y + 206, C_MUTED);
        if (trans_confirm_backup) {
            char msg[180];
            utf8_to_latin1(lang == LANG_PT
                          ? "BLOQUEADO: backup _bkp já existe. Preserve-o!"
                          : "BLOCKED: a _bkp backup already exists.", msg, sizeof(msg));
            body_draw(msg, x + 28, y + 250, C_PROGRESS_LOW);
        } else if (!trans_confirm_plugin) {
            body_draw(lang == LANG_PT ? "Ative o rePatch no taiHEN e reinicie o Vita."
                                     : "Enable rePatch in taiHEN and restart your Vita.",
                      x + 28, y + 250, C_MUTED);
        } else {
            body_draw(lang == LANG_PT ? "Destino: ux0:rePatch/"
                                     : "Destination: ux0:rePatch/",
                      x + 28, y + 250, C_MUTED);
        }
    }

    const int blocked = trans_confirm_backup || trans_confirm_base_missing ||
                        (!trans_confirm_app && !trans_confirm_plugin);
    const float by = y + 316;
    draw_translation_action(x + 32, by, 328, TXT(S_TRANS_CONFIRM), 1, blocked);
    draw_translation_action(x + 400, by, 328, TXT(S_TRANS_CANCEL), 0, 0);
}

static void draw_plugin_advice(void) {
    if (plugin_advice_post < 0) return;
    const float x = 100, y = 64, w = 760, h = 410;
    vita2d_draw_rectangle(0, 0, SCR_W, SCR_H, RGBA8(0, 0, 0, 0xE5));
    halo_draw(x - 16, y - 16, w + 32, h + 32, 0, 1);
    vita2d_draw_rectangle(x, y, w, h, C_PANEL);
    frame_rect(x, y, w, h, 4, C_WHITE);
    const Download *d=&g_posts[plugin_advice_post].dls[plugin_advice_dl];
    const int tai=d->type==DL_TAI_PLUGIN;
    const char *heading=tai
        ? (lang==LANG_PT ? "INSTALAR TOUCHONVITA?" : "INSTALL TOUCHONVITA?")
        : TXT(S_CAP_TITLE);
    title_draw(heading,x+(w-title_width(heading,2))/2,y+26,2,C_WHITE);
    vita2d_draw_rectangle(x+28,y+76,w-56,4,C_LINE);
    if(tai){
        body_draw(lang==LANG_PT ? "O jogo Darkest Dungeon deve estar instalado."
                                : "Darkest Dungeon must be installed.",x+28,y+115,C_GOLD);
        body_draw(lang==LANG_PT ? "Backup de config.txt e plugin anterior."
                                : "Backup of config.txt and previous plugin.",x+28,y+164,C_WHITE);
        body_draw(lang==LANG_PT ? "Instala ur0:tai e ux0:data. Reinicie o Vita."
                                : "Installs to ur0:tai and ux0:data. Reboot PS Vita.",x+28,y+217,C_MUTED);
    }else{
        char line[160];
        snprintf(line,sizeof(line),"%s%s",TXT(S_CAP_RECOMMEND),d->advisory_plugin);
        body_draw(line,x+28,y+115,C_GOLD);
        body_draw(TXT(S_CAP_CPU),x+28,y+164,C_WHITE);
        body_draw(TXT(S_CAP_OPTIONAL),x+28,y+217,C_MUTED);
    }
    draw_translation_action(x + 32, y + 316, 328, TXT(S_TRANS_CONFIRM), 1, 0);
    draw_translation_action(x + 400, y + 316, 328, TXT(S_TRANS_CANCEL), 0, 0);
}

/* Patch notes are a one-time modal AFTER the welcome animation. */
static void draw_update_notes(void) {
    if (!update_notes_visible) return;
    float p = fmaxf(0.f, fminf(1.f, (float)((now_s - update_notes_t0) / .25)));
    p = 1.f - powf(1.f - p, 3.f);
    const float x = 110, y = 74 + 18 * (1 - p), w = 740, h = 396;
    vita2d_draw_rectangle(0, 0, SCR_W, SCR_H, RGBA8(0, 0, 0, (int)(0xDD * p)));
    halo_draw(x - 16, y - 16, w + 32, h + 32, 1, p);
    vita2d_draw_rectangle(x, y, w, h, col_alpha(C_PANEL, p));
    frame_rect(x, y, w, h, 4, col_alpha(C_WHITE, p));
    char headline[100], line[220], details[3][220];
    utf8_to_latin1(lang == LANG_PT ? "NOTAS DA ATUALIZAÇÃO" : "UPDATE NOTES",
                   headline, sizeof(headline));
    title_draw(headline, x + (w - title_width(headline, 2)) / 2,
               y + 34, 2, col_alpha(C_WHITE, p));
    snprintf(line, sizeof(line), "Wolff's Store v%s", WS_VERSION);
    body_draw(line, x + (w - body_width(line)) / 2, y + 79, col_alpha(C_GOLD, p));
    vita2d_draw_rectangle(x + 32, y + 111, w - 64, 4, col_alpha(C_LINE, p));
    static const char *pt[] = {
        "Updater reutilizado sem apagar a bolha.",
        "Corrigido o título Lançamentos no Vita.",
        "Novidade Pokémon Emerald v0.5.8 no site."};
    static const char *en[] = {
        "Updater reused without deleting its bubble.",
        "Fixed the Releases title on PS Vita.",
        "Pokémon Emerald v0.5.8 news on the site."};
    for (int i = 0; i < 3; i++) {
        utf8_to_latin1(lang == LANG_PT ? pt[i] : en[i], details[i], sizeof(details[i]));
        body_draw(details[i], x + 40, y + 138 + i * 52, col_alpha(C_WHITE, p));
    }
    vita2d_draw_rectangle(x + 32, y + 324, w - 64, 4, col_alpha(C_LINE, p));
    const char *close = lang == LANG_PT ? "X  FECHAR" : "X  CLOSE";
    title_draw(close, x + (w - title_width(close, 2)) / 2,
               y + 347, 2, col_alpha(C_WHITE, p));
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
    poll_box_tex = vita2d_load_PNG_file("app0:data/polls/caixa.png");
    poll_paper_tex = vita2d_load_PNG_file("app0:data/polls/papel.png");
    if (poll_box_tex) vita2d_texture_set_filters(poll_box_tex,
        SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_POINT);
    if (poll_paper_tex) vita2d_texture_set_filters(poll_paper_tex,
        SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_POINT);
    audio_init();
    settings_load();
    reg_load();
    sceSysmoduleLoadModule(SCE_SYSMODULE_IME);
    SceCommonDialogConfigParam cdc;
    sceCommonDialogConfigParamInit(&cdc);
    sceCommonDialogSetConfigParam(&cdc);
    /* catálogo: o último sincronizado (cartão) ou o que veio no VPK */
    int cached = store_load_cache(g_posts, MAX_POSTS, &app_rel);
    /* Always refresh before the question/intro, even when cache exists.
     * On network failure the existing cache or bundled catalog is retained. */
    first_sync = 1;
    if (cached > 0) g_nposts = cached;
    else catalog_load("app0:data/catalog.txt");
    feat = featured_post_for_device();
    load_covers();
    make_mosaic();
    /* Defer helper removal only until it has finished exiting, not until a
     * SECOND launch of Store. The updater writes the marker after promotion. */
    fs_mkdir_p(WS_DATA);
    {
        char *seen = fs_read(WS_DATA "/journal_seen_slug.txt", NULL);
        if (seen) {
            snprintf(journal_seen_slug, sizeof(journal_seen_slug), "%.47s", seen);
            free(seen);
        }
    }
    {
        int last_len = 0;
        char *last = fs_read(WS_DATA "/last_started_version.txt", &last_len);
        free(last);
        fs_write(WS_DATA "/last_started_version.txt", WS_VERSION, (int)strlen(WS_VERSION));
        char *seen = fs_read(WS_DATA "/release_notes_seen.txt", NULL);
        if (last_len <= 0 && !seen) {
            /* Fresh Store installation is not an update. */
            fs_write(WS_DATA "/release_notes_seen.txt", WS_VERSION, (int)strlen(WS_VERSION));
        } else if (!seen || strcmp(seen, WS_VERSION) != 0) {
            update_notes_pending = 1;
        }
        free(seen);
    }
    /* Existing v1.0.10 updater may have left a delete-on-boot marker.
     * Do not remove its LiveArea title from the foreground Store process. */
    updater_retire_marker_at_startup();

    /* sobras de uma atualização do próprio app */
    fs_rm_rf(WS_PKG_SELF);
    fs_rm_rf(WS_PKG_UPD);
    jobs_init();
    start_sync(0);

    double t_start = clock_s();
    app_clock_t0 = t_start;
    now_s = 0;
    first_sync_t0 = now_s;
    view_t0 = 0;
    build_home();
    /* Title cues begin only after apply_sync has applied latest featured
     * metadata and the featured artwork/mosaic was rebuilt. */

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
        } else if (update_notes_visible) {
            if (down & (SCE_CTRL_CROSS | SCE_CTRL_CIRCLE | SCE_CTRL_START))
                close_update_notes();
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
        } else if (plugin_advice_post >= 0) {
            if (down & SCE_CTRL_CIRCLE) close_plugin_advice(0);
            else if (down & SCE_CTRL_CROSS) close_plugin_advice(1);
        } else if (trans_confirm_post >= 0) {
            if (down & SCE_CTRL_CIRCLE) close_translation_confirm(0);
            else if (down & SCE_CTRL_CROSS) close_translation_confirm(1);
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
            if ((started || first_sync) && !vid_open && !settings_open && trans_confirm_post < 0 && plugin_advice_post < 0) {
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
            if (dragged && viewer < 0 && !vid_open && !settings_open && !snd_menu && !update_notes_visible && trans_confirm_post < 0 && plugin_advice_post < 0) {
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
                else if (update_notes_visible) close_update_notes();
                else if (plugin_advice_post >= 0) {
                    if (t_y0 >= 380 && t_y0 < 428) {
                        if (t_x0 >= 132 && t_x0 < 460) close_plugin_advice(1);
                        else if (t_x0 >= 500 && t_x0 < 828) close_plugin_advice(0);
                    } else if (t_y0 < 70 || t_y0 > 475) close_plugin_advice(0);
                } else if (trans_confirm_post >= 0) {
                    if (t_y0 >= 380 && t_y0 < 428) {
                        if (t_x0 >= 132 && t_x0 < 460) close_translation_confirm(1);
                        else if (t_x0 >= 500 && t_x0 < 828) close_translation_confirm(0);
                    } else if (t_y0 < 70 || t_y0 > 475) close_translation_confirm(0);
                } else if (snd_menu_touch(t_x0, t_y0)) {
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
            draw_poll_vote_animation();
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
        draw_translation_confirm();
        draw_plugin_advice();
        draw_update_notes();
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
