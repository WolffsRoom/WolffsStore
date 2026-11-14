#ifndef WS_GFX_H
#define WS_GFX_H

#include <vita2d.h>

#define SCR_W 960
#define SCR_H 544

/* Paleta do site */
#define C_BG     RGBA8(0x06, 0x06, 0x06, 0xFF)
#define C_PANEL  RGBA8(0x0E, 0x0E, 0x0E, 0xFF)
#define C_DARK   RGBA8(0x0B, 0x0B, 0x0B, 0xFF)
#define C_GOLD   RGBA8(0xF4, 0xC4, 0x30, 0xFF)
#define C_BTN    RGBA8(0x14, 0x14, 0x14, 0xFF)
#define C_LINE   RGBA8(0x2E, 0x2E, 0x2E, 0xFF)
#define C_BORDER RGBA8(0x5A, 0x5A, 0x5A, 0xFF)
#define C_BTNBRD RGBA8(0x8A, 0x8A, 0x8A, 0xFF)
#define C_MUTED  RGBA8(0xA8, 0xA8, 0xA8, 0xFF)
#define C_SOFT   RGBA8(0xC4, 0xC4, 0xC4, 0xFF)
#define C_TEXT   RGBA8(0xD6, 0xD6, 0xD6, 0xFF)
#define C_WHITE  RGBA8(0xFF, 0xFF, 0xFF, 0xFF)
#define C_SHADOW RGBA8(0x3A, 0x3A, 0x3A, 0xFF)
#define C_SH1    RGBA8(0x4A, 0x4A, 0x4A, 0xFF)
#define C_SH2    RGBA8(0x22, 0x22, 0x22, 0xFF)

static inline unsigned col_alpha(unsigned c, float a) {
    if (a >= 1.f) return c;
    if (a <= 0.f) return c & 0x00FFFFFF;
    unsigned ca = (c >> 24) & 0xFF;
    return (c & 0x00FFFFFF) | ((unsigned)(ca * a) << 24);
}

vita2d_texture *tex_load(const char *path, int pixel);
vita2d_texture *tex_from_rgba(const unsigned char *rgba, int w, int h, int pixel);
unsigned char *wrt_read(const char *path, int *w, int *h); /* RGBA8, malloc */
void tex_draw(vita2d_texture *t, float x, float y, float w, float h, unsigned tint);

int gfx_init(void);

/* Press Start 2P: 8 px por célula; scale 2 = 16 px, 4 = 32 px */
int title_width(const char *s, int scale);
void title_draw(const char *s, float x, float y, int scale, unsigned color);
/* Escala livre (efeito de TV desligando), a partir do canto (x, y) */
void title_draw_f(const char *s, float x, float y, float sx, float sy, unsigned color);
void title_draw_shadow(const char *s, float x, float y, int scale, unsigned color, unsigned shadow);
/* Pixelify Sans 18 px, como o texto do site */
#define BODY_LINE 26
int body_width(const char *s);
void body_draw(const char *s, float x, float y, unsigned color);
void body_draw_bold(const char *s, float x, float y, unsigned color);

/* Quebra em linhas que cabem em maxw. Devolve o número de linhas;
 * cada linha vai para out[i] (ponteiro para buf). */
int wrap_body(const char *s, int maxw, char **out, int maxlines, char *buf, int bufsz);
int wrap_title(const char *s, int scale, int maxw, char **out, int maxlines, char *buf, int bufsz);
void upper_latin1(char *s);

void frame_rect(float x, float y, float w, float h, float t, unsigned c);
/* Moldura de pixels do site (border-image com repeat). big: 24 px, senão 12 px */
void halo_draw(float x, float y, float w, float h, int big, float alpha);
/* Transição em dither do site (w75/w50/w25); stage 0 = sólido .. 4 = nada */
void dither_fill(int stage, float x, float y, float w, float h, unsigned color);
void paw_draw(float x, float y, float scale, float alpha);

#endif
