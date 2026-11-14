#include "gfx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static vita2d_texture *t_title, *t_body, *t_paw, *t_halo24, *t_halo12, *t_dither[3];
static unsigned char body_w[224];

unsigned char *wrt_read(const char *path, int *w, int *h) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    unsigned char hd[8];
    if (fread(hd, 1, 8, f) != 8 || memcmp(hd, "WRT1", 4)) {
        fclose(f);
        return NULL;
    }
    *w = hd[4] | hd[5] << 8;
    *h = hd[6] | hd[7] << 8;
    size_t n = (size_t)*w * *h * 4;
    unsigned char *px = malloc(n);
    if (px && fread(px, 1, n, f) != n) {
        free(px);
        px = NULL;
    }
    fclose(f);
    return px;
}

vita2d_texture *tex_from_rgba(const unsigned char *rgba, int w, int h, int pixel) {
    vita2d_texture *t = vita2d_create_empty_texture_format(w, h, SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ABGR);
    if (!t) return NULL;
    unsigned char *dst = vita2d_texture_get_datap(t);
    unsigned stride = vita2d_texture_get_stride(t);
    for (int y = 0; y < h; y++) memcpy(dst + y * stride, rgba + (size_t)y * w * 4, (size_t)w * 4);
    if (pixel) vita2d_texture_set_filters(t, SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_POINT);
    else vita2d_texture_set_filters(t, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);
    return t;
}

vita2d_texture *tex_load(const char *path, int pixel) {
    int w, h;
    unsigned char *px = wrt_read(path, &w, &h);
    if (!px) return NULL;
    vita2d_texture *t = tex_from_rgba(px, w, h, pixel);
    free(px);
    return t;
}

void tex_draw(vita2d_texture *t, float x, float y, float w, float h, unsigned tint) {
    if (!t) return;
    float tw = vita2d_texture_get_width(t), th = vita2d_texture_get_height(t);
    vita2d_draw_texture_tint_part_scale(t, x, y, 0, 0, tw, th, w / tw, h / th, tint);
}

int gfx_init(void) {
    t_title = tex_load("app0:data/tex/font_title.wrt", 1);
    t_body = tex_load("app0:data/tex/font_body.wrt", 1);
    t_paw = tex_load("app0:data/tex/paw4.wrt", 1);
    t_halo24 = tex_load("app0:data/tex/halo24.wrt", 1);
    t_halo12 = tex_load("app0:data/tex/halo12.wrt", 1);
    t_dither[0] = tex_load("app0:data/tex/w75.wrt", 1);
    t_dither[1] = tex_load("app0:data/tex/w50.wrt", 1);
    t_dither[2] = tex_load("app0:data/tex/w25.wrt", 1);
    FILE *f = fopen("app0:data/tex/font_body.wid", "rb");
    if (f) {
        if (fread(body_w, 1, sizeof(body_w), f) != sizeof(body_w)) memset(body_w, 10, sizeof(body_w));
        fclose(f);
    } else {
        memset(body_w, 10, sizeof(body_w));
    }
    return t_title && t_body ? 0 : -1;
}

/* ------------------------------------------------------------ texto */

static int glyph(unsigned char c) { return c < 32 ? 0 : c - 32; }

int title_width(const char *s, int scale) { return (int)strlen(s) * 8 * scale; }

void title_draw(const char *s, float x, float y, int scale, unsigned color) {
    for (const unsigned char *p = (const unsigned char *)s; *p; p++, x += 8 * scale) {
        if (*p == ' ') continue;
        int g = glyph(*p);
        /* célula de 8x11: 3 linhas acima da letra para os acentos das maiúsculas */
        vita2d_draw_texture_tint_part_scale(t_title, x, y - 3 * scale, (g % 16) * 8, (g / 16) * 11, 8, 11, scale, scale, color);
    }
}

void title_draw_f(const char *s, float x, float y, float sx, float sy, unsigned color) {
    for (const unsigned char *p = (const unsigned char *)s; *p; p++, x += 8 * sx) {
        if (*p == ' ') continue;
        int g = glyph(*p);
        vita2d_draw_texture_tint_part_scale(t_title, x, y - 3 * sy, (g % 16) * 8, (g / 16) * 11, 8, 11, sx, sy, color);
    }
}

void title_draw_shadow(const char *s, float x, float y, int scale, unsigned color, unsigned shadow) {
    title_draw(s, x + 4, y + 4, scale, shadow);
    title_draw(s, x, y, scale, color);
}

int body_width(const char *s) {
    int w = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p; p++) w += body_w[glyph(*p)];
    return w;
}

void body_draw(const char *s, float x, float y, unsigned color) {
    for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
        int g = glyph(*p);
        if (*p != ' ')
            vita2d_draw_texture_tint_part_scale(t_body, x - 1, y, (g % 16) * 24, (g / 16) * 24, 24, 24, 1, 1, color);
        x += body_w[g];
    }
}

void body_draw_bold(const char *s, float x, float y, unsigned color) {
    body_draw(s, x, y, color);
    body_draw(s, x + 1, y, color);
}

void upper_latin1(char *s) {
    for (unsigned char *p = (unsigned char *)s; *p; p++) {
        if (*p >= 'a' && *p <= 'z') *p -= 32;
        else if (*p >= 0xE0 && *p <= 0xFE && *p != 0xF7) *p -= 32;
    }
}

typedef int (*measure_fn)(const char *s, int arg);
static int m_body(const char *s, int arg) { (void)arg; return body_width(s); }
static int m_title(const char *s, int arg) { return title_width(s, arg); }

static int wrap(const char *s, int maxw, char **out, int maxlines, char *buf, int bufsz, measure_fn m, int arg) {
    int n = 0, used = 0;
    const char *p = s;
    while (*p == ' ') p++;
    while (*p && n < maxlines) {
        /* junta palavras enquanto couberem; a primeira entra sempre */
        const char *best = NULL, *q = p;
        char tmp[512];
        for (;;) {
            const char *e = q;
            while (*e && *e != ' ') e++;
            int len = (int)(e - p);
            if (len >= (int)sizeof(tmp)) len = sizeof(tmp) - 1;
            memcpy(tmp, p, len);
            tmp[len] = 0;
            if (best && m(tmp, arg) > maxw) break;
            best = e;
            if (!*e) break;
            q = e + 1;
        }
        int len = (int)(best - p);
        /* palavra sozinha maior que a linha: corta por letra */
        if (len > 0 && len < 512) {
            char tmp[512];
            memcpy(tmp, p, len);
            tmp[len] = 0;
            while (len > 1 && m(tmp, arg) > maxw) tmp[--len] = 0;
            best = p + len;
        }
        if (used + len + 4 > bufsz) break;
        memcpy(buf + used, p, len);
        buf[used + len] = 0;
        out[n++] = buf + used;
        used += len + 1;
        p = best;
        while (*p == ' ') p++;
    }
    /* sobrou texto: reticências na última linha */
    if (*p && n > 0) {
        char *last = out[n - 1];
        int len = (int)strlen(last);
        char tmp[520];
        for (;;) {
            snprintf(tmp, sizeof(tmp), "%.*s...", len, last);
            if (m(tmp, arg) <= maxw || len == 0) break;
            while (len > 0 && last[len - 1] != ' ') len--;
            while (len > 0 && last[len - 1] == ' ') len--;
        }
        if (last + strlen(tmp) < buf + bufsz) {
            memcpy(last, tmp, strlen(tmp) + 1);
            /* a linha cresceu: só é seguro porque é a última escrita no buffer */
        }
    }
    return n;
}

int wrap_body(const char *s, int maxw, char **out, int maxlines, char *buf, int bufsz) {
    return wrap(s, maxw, out, maxlines, buf, bufsz, m_body, 0);
}

int wrap_title(const char *s, int scale, int maxw, char **out, int maxlines, char *buf, int bufsz) {
    return wrap(s, maxw, out, maxlines, buf, bufsz, m_title, scale);
}

/* ------------------------------------------------------------ formas */

void frame_rect(float x, float y, float w, float h, float t, unsigned c) {
    vita2d_draw_rectangle(x, y, w, t, c);
    vita2d_draw_rectangle(x, y + h - t, w, t, c);
    vita2d_draw_rectangle(x, y + t, t, h - 2 * t, c);
    vita2d_draw_rectangle(x + w - t, y + t, t, h - 2 * t, c);
}

/* Uma borda repetida: tiles inteiros centrados, cortados nas pontas (CSS repeat). */
static void edge(vita2d_texture *t, int sl, int mid, int horiz, float x, float y, float len, int sx, int sy, unsigned tint) {
    if (len <= 0) return;
    int n = (int)(len / mid) + 2;
    float start = (len - n * mid) / 2.f;
    for (int i = 0; i < n; i++) {
        float a = start + i * mid, b = a + mid;
        float ca = a < 0 ? 0 : a, cb = b > len ? len : b;
        if (cb <= ca) continue;
        float off = ca - a;
        if (horiz)
            vita2d_draw_texture_tint_part_scale(t, x + ca, y, sx + off, sy, cb - ca, sl, 1, 1, tint);
        else
            vita2d_draw_texture_tint_part_scale(t, x, y + ca, sx, sy + off, sl, cb - ca, 1, 1, tint);
    }
}

void halo_draw(float x, float y, float w, float h, int big, float alpha) {
    vita2d_texture *t = big ? t_halo24 : t_halo12;
    if (!t || alpha <= 0) return;
    int sl = big ? 24 : 12, T = (int)vita2d_texture_get_width(t), mid = T - 2 * sl;
    unsigned tint = col_alpha(C_WHITE, alpha);
    /* cantos */
    vita2d_draw_texture_tint_part_scale(t, x, y, 0, 0, sl, sl, 1, 1, tint);
    vita2d_draw_texture_tint_part_scale(t, x + w - sl, y, T - sl, 0, sl, sl, 1, 1, tint);
    vita2d_draw_texture_tint_part_scale(t, x, y + h - sl, 0, T - sl, sl, sl, 1, 1, tint);
    vita2d_draw_texture_tint_part_scale(t, x + w - sl, y + h - sl, T - sl, T - sl, sl, sl, 1, 1, tint);
    edge(t, sl, mid, 1, x + sl, y, w - 2 * sl, sl, 0, tint);
    edge(t, sl, mid, 1, x + sl, y + h - sl, w - 2 * sl, sl, T - sl, tint);
    edge(t, sl, mid, 0, x, y + sl, h - 2 * sl, 0, sl, tint);
    edge(t, sl, mid, 0, x + w - sl, y + sl, h - 2 * sl, T - sl, sl, tint);
}

void dither_fill(int stage, float x, float y, float w, float h, unsigned color) {
    if (stage <= 0) {
        vita2d_draw_rectangle(x, y, w, h, color);
        return;
    }
    if (stage > 3 || !t_dither[stage - 1]) return;
    vita2d_texture *t = t_dither[stage - 1];
    int tw = vita2d_texture_get_width(t), th = vita2d_texture_get_height(t);
    for (float yy = y; yy < y + h; yy += th)
        for (float xx = x; xx < x + w; xx += tw)
            vita2d_draw_texture_tint_part_scale(t, xx, yy, 0, 0, tw, th, 1, 1, color);
}

void paw_draw(float x, float y, float scale, float alpha) {
    if (!t_paw) return;
    float w = vita2d_texture_get_width(t_paw), h = vita2d_texture_get_height(t_paw);
    vita2d_draw_texture_tint_part_scale(t_paw, x, y, 0, 0, w, h, scale, scale, col_alpha(C_WHITE, alpha));
}
