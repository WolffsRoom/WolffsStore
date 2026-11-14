#include "audio.h"

#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define RATE 48000
#define GRAIN 256
#define VOICES 48
#define ECHO_LEN (RATE * 11 / 100) /* 0,11 s, como o delay do site */

enum { W_P12, W_P25, W_P50, W_TRI, W_NOISE };

typedef struct {
    volatile int state; /* 0 livre, 1 pronto para tocar, 2 tocando */
    float f0, f1, vol;
    int wait, len, t;   /* em amostras */
    int wave;
    float phase;
    unsigned rng;
} Voice;

/* [freqInicial, freqFinal, início(s), duração(s), onda, volume] */
typedef struct { float f0, f1, st, dur; int wave; float vol; } Note;

static Voice voices[VOICES];
static volatile int enabled = 1;
static volatile int music_on = 1;
/* Música só pode ser ouvida depois da primeira apresentação da tela inicial. */
static volatile int music_screen_ready = 0;

/* ------------------------------------------------------------------ música de fundo
 * Misteriosa, em lá menor, 72 BPM, 16 compassos (~53 s): Am F Dm E | Am Bb Dm E, duas vezes.
 * Baixo em triângulo bem grave, arpejo lento e abafado (pulso 12,5%) e melodia esparsa em
 * triângulo com o sol# (sensível) e o si bemol (napolitano) para dar o clima sombrio. Na segunda
 * volta a melodia some nos 4 primeiros compassos. O E do fim puxa de volta para o Am do começo e
 * o sequenciador é contínuo (posição em amostras), então o loop não tem emenda. */
#define M_STEP (RATE * 60 / 72 / 4)   /* semicolcheia */
#define M_STEPS (16 * 16)
#define M_VOICES 12
static Voice mvoices[M_VOICES];
static unsigned mpos;                 /* amostra dentro do loop */

typedef struct { short step; signed char note, len, wave; float vol; } MEv;
static MEv mev[400];
static int nmev;

static float mtof(int m) { return 440.f * powf(2.f, (m - 69) / 12.f); }

static void mev_add(int step, int note, int len, int wave, float vol) {
    if (nmev < (int)(sizeof(mev) / sizeof(mev[0]))) mev[nmev++] = (MEv){(short)step, (signed char)note, (signed char)len, (signed char)wave, vol};
}

static void music_build(void) {
    /* Am F Dm E | Am Bb Dm E */
    static const int bass[8] = {33, 29, 38, 40, 33, 34, 38, 40};
    static const int chord[8][3] = {{57, 60, 64}, {53, 57, 60}, {50, 53, 57}, {52, 56, 59},
                                    {57, 60, 64}, {53, 58, 62}, {50, 53, 57}, {52, 56, 59}};
    /* melodia: {compasso, passo, nota, duração} */
    static const int mel[][4] = {
        {0, 0, 72, 6}, {0, 6, 71, 2}, {0, 8, 69, 8},
        {1, 0, 69, 4}, {1, 4, 72, 4}, {1, 8, 77, 8},
        {2, 0, 76, 6}, {2, 6, 74, 2}, {2, 8, 72, 8},
        {3, 0, 71, 8}, {3, 8, 68, 8},
        {4, 0, 69, 12}, {4, 12, 64, 4},
        {5, 0, 70, 6}, {5, 6, 69, 2}, {5, 8, 65, 8},
        {6, 0, 65, 4}, {6, 4, 69, 4}, {6, 8, 74, 8},
        {7, 0, 71, 4}, {7, 4, 72, 4}, {7, 8, 71, 4}, {7, 12, 68, 4},
    };
    static const int arp[8] = {0, 1, 2, 1, 0, 1, 2, 1};
    nmev = 0;
    for (int rep = 0; rep < 2; rep++)
        for (int b = 0; b < 8; b++) {
            int base = (rep * 8 + b) * 16;
            mev_add(base, bass[b], 15, W_TRI, .70f);          /* nota longa, o compasso todo */
            mev_add(base + 12, bass[b] + 12, 3, W_TRI, .22f);  /* eco da oitava no fim do compasso */
            for (int s = 0; s < 8; s++) mev_add(base + s * 2, chord[b][arp[s]], 2, W_P12, .07f);
        }
    for (int rep = 0; rep < 2; rep++)
        for (unsigned i = 0; i < sizeof(mel) / sizeof(mel[0]); i++) {
            if (rep == 1 && mel[i][0] < 4) continue; /* respiro na segunda volta */
            mev_add((rep * 8 + mel[i][0]) * 16 + mel[i][1], mel[i][2], mel[i][3], W_TRI, .42f);
        }
}

static void music_step(int step) {
    for (int e = 0; e < nmev; e++) {
        if (mev[e].step != step) continue;
        for (int k = 0; k < M_VOICES; k++) {
            Voice *v = &mvoices[k];
            if (v->state) continue;
            v->f0 = v->f1 = mtof(mev[e].note);
            v->vol = mev[e].vol;
            v->wait = 0;
            v->len = mev[e].len * M_STEP - M_STEP / 4; /* solta antes da próxima nota */
            v->t = 0;
            v->wave = mev[e].wave;
            v->phase = 0;
            v->state = 2;
            break;
        }
    }
}
static float echo[ECHO_LEN];
static int echo_pos;

static float osc(Voice *v) {
    float f = v->f0;
    if (v->f1 != v->f0 && v->len > 0) f = v->f0 * powf(v->f1 / v->f0, (float)v->t / v->len);
    float s;
    switch (v->wave) {
    case W_NOISE:
        v->rng = v->rng * 1103515245u + 12345u;
        s = ((v->rng >> 16) & 0x7fff) / 16383.5f - 1.f;
        s *= 1.f - (float)v->t / v->len;
        break;
    case W_TRI:
        s = v->phase < .5f ? v->phase * 4.f - 1.f : 3.f - v->phase * 4.f;
        break;
    default: {
        float duty = v->wave == W_P12 ? .125f : v->wave == W_P25 ? .25f : .5f;
        s = v->phase < duty ? 1.f : -1.f;
        s -= 2.f * duty - 1.f; /* sem componente DC, como a PeriodicWave */
        s *= .6f;
    }
    }
    v->phase += f / RATE;
    if (v->phase >= 1.f) v->phase -= floorf(v->phase);
    /* envelope: ataque de 4 ms, sustenta até 60%, solta até o fim */
    float a = (float)v->t / RATE, d = (float)v->len / RATE, g;
    if (a < .004f) g = a / .004f;
    else if (a < d * .6f) g = 1.f;
    else g = 1.f - (a - d * .6f) / (d * .4f);
    if (g < 0) g = 0;
    return s * g * v->vol;
}

static int audio_thread(SceSize args, void *argp) {
    (void)args;
    (void)argp;
    int port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, GRAIN, RATE, SCE_AUDIO_OUT_MODE_MONO);
    if (port < 0) return 0;
    int vol[2] = {SCE_AUDIO_VOLUME_0DB, SCE_AUDIO_VOLUME_0DB};
    sceAudioOutSetVolume(port, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, vol);
    static short buf[2][GRAIN];
    int cur = 0;
    for (;;) {
        short *out = buf[cur];
        for (int i = 0; i < GRAIN; i++) {
            float mix = 0;
            for (int k = 0; k < VOICES; k++) {
                Voice *v = &voices[k];
                if (v->state == 1) v->state = 2;
                if (v->state != 2) continue;
                if (v->wait > 0) { v->wait--; continue; }
                mix += osc(v);
                if (++v->t >= v->len) v->state = 0;
            }
            /* música: sequenciador contínuo, sem emenda no loop */
            if (music_on && music_screen_ready) {
                if (mpos % M_STEP == 0) music_step((int)(mpos / M_STEP));
                if (++mpos >= (unsigned)(M_STEPS * M_STEP)) mpos = 0;
            }
            for (int k = 0; k < M_VOICES; k++) {
                Voice *v = &mvoices[k];
                if (v->state != 2) continue;
                mix += osc(v) * .55f; /* baixinha */
                if (++v->t >= v->len) v->state = 0;
            }
            float dry = mix * .16f;                  /* ganho geral */
            float wet = echo[echo_pos];
            echo[echo_pos] = dry + wet * .28f;       /* realimentação do eco */
            echo_pos = (echo_pos + 1) % ECHO_LEN;
            float s = dry + wet * .35f;
            if (s > 1.f) s = 1.f;
            if (s < -1.f) s = -1.f;
            out[i] = (short)(s * 32000.f);
        }
        sceAudioOutOutput(port, out);
        cur ^= 1;
    }
    return 0;
}

void audio_set_music(int on) {
    if (on && !music_on) mpos = 0; /* volta do começo da música */
    if (!on)
        for (int k = 0; k < M_VOICES; k++) mvoices[k].state = 0;
    music_on = on;
}
int audio_music(void) { return music_on; }

/* Chamar somente após vita2d_swap_buffers() apresentar a tela inicial.
 * A configuração de música permanece intacta; o sequenciador parte do início. */
void audio_start_music_after_first_frame(void) {
    if (music_screen_ready) return;
    mpos = 0;
    __sync_synchronize();
    music_screen_ready = 1;
}

void audio_init(void) {
    music_build();
    SceUID th = sceKernelCreateThread("ws_audio", audio_thread, 0x10000100 - 10, 0x4000, 0, 0, NULL);
    if (th >= 0) sceKernelStartThread(th, 0, NULL);
}

void audio_set_enabled(int on) { enabled = on; }
int audio_enabled(void) { return enabled; }

static void play(const Note *n, int count, int force) {
    if (!enabled && !force) return;
    for (int i = 0; i < count; i++) {
        for (int k = 0; k < VOICES; k++) {
            Voice *v = &voices[k];
            if (v->state != 0) continue;
            v->f0 = n[i].f0;
            v->f1 = n[i].f1;
            v->vol = n[i].vol;
            v->wait = (int)(n[i].st * RATE) + RATE / 100;
            v->len = (int)(n[i].dur * RATE);
            if (v->len < 1) v->len = 1;
            v->t = 0;
            v->wave = n[i].wave;
            v->phase = 0;
            v->rng = (unsigned)rand();
            __sync_synchronize();
            v->state = 1;
            break;
        }
    }
}

#define C5 523
#define D5 587
#define E5 659
#define G5 784
#define A5 880
#define C6 1047
#define E6 1319
#define G6 1568

#define N(...) { __VA_ARGS__ }
#define PLAY(...) do { static const Note s_[] = { __VA_ARGS__ }; play(s_, sizeof(s_) / sizeof(s_[0]), 0); } while (0)

void sfx(const char *k) {
    if (!strcmp(k, "hover")) PLAY(N(1760, 1760, 0, .018f, W_P12, .35f));
    else if (!strcmp(k, "open")) PLAY(N(988, 988, 0, .05f, W_P25, 1), N(1319, 1319, .05f, .12f, W_P25, 1));
    else if (!strcmp(k, "back")) PLAY(N(784, 523, 0, .08f, W_P50, .9f), N(392, 262, .07f, .1f, W_P50, .9f));
    else if (!strcmp(k, "tick")) PLAY(N(1568, 1568, 0, .03f, W_P12, .8f));
    else if (!strcmp(k, "peek")) PLAY(N(C5, C5, 0, .04f, W_P25, 1), N(E5, E5, .04f, .04f, W_P25, 1), N(G5, G5, .08f, .04f, W_P25, 1),
                                      N(C6, C6, .12f, .1f, W_P25, 1), N(131, 131, 0, .22f, W_TRI, 1.4f));
    else if (!strcmp(k, "confirm")) PLAY(N(C6, C6, 0, .06f, W_P25, 1), N(G6, G6, .06f, .14f, W_P25, 1), N(0, 0, 0, .05f, W_NOISE, .5f),
                                         N(196, 196, 0, .2f, W_TRI, 1.4f));
    else if (!strcmp(k, "up")) PLAY(N(C5, G6, 0, .18f, W_P25, .9f), N(131, 262, 0, .18f, W_TRI, 1.2f));
    else if (!strcmp(k, "on")) {
        static const Note s[] = {N(C5, C5, 0, .06f, W_P25, 1), N(E5, E5, .06f, .06f, W_P25, 1), N(G5, G5, .12f, .06f, W_P25, 1),
                                 N(C6, G6, .18f, .14f, W_P25, 1), N(131, 131, 0, .3f, W_TRI, 1.6f)};
        play(s, 5, 1); /* toca mesmo com o som acabando de ligar */
    } else if (!strcmp(k, "boot")) PLAY(N(262, 262, 0, .06f, W_TRI, 1.6f), N(0, 0, 0, .03f, W_NOISE, .35f));
    else if (!strcmp(k, "off")) PLAY(N(1047, 131, 0, .24f, W_P50, .8f), N(0, 0, .02f, .14f, W_NOISE, .3f));
    else if (!strcmp(k, "tvon")) PLAY(N(0, 0, 0, .28f, W_NOISE, .6f), N(60, 1200, 0, .36f, W_P50, .8f), N(55, 55, 0, .45f, W_TRI, 1.5f));
    else if (!strcmp(k, "type")) {
        static const float r[3] = {C5, D5, E5};
        float f = r[rand() % 3];
        Note n = {f, f, 0, .028f, W_P12, .6f};
        play(&n, 1, 0);
    } else if (!strcmp(k, "home")) /* volta ao início: chiado de TV, varredura e arpejo subindo */
        PLAY(N(0, 0, 0, .12f, W_NOISE, .45f), N(80, 900, 0, .16f, W_P50, .6f), N(98, 98, 0, .2f, W_TRI, 1.3f),
             N(G5, G5, .16f, .05f, W_P25, .9f), N(C6, C6, .21f, .05f, W_P25, .9f), N(E6, E6, .26f, .05f, W_P25, .9f),
             N(G6, G6, .31f, .14f, W_P25, .9f), N(131, 262, .16f, .3f, W_TRI, 1.3f));
    else if (!strcmp(k, "ready")) PLAY(N(E5, E5, 0, .08f, W_P25, 1), N(G5, G5, .08f, .08f, W_P25, 1), N(C6, C6, .16f, .08f, W_P25, 1),
                                         N(E6, E6, .24f, .22f, W_P25, 1), N(165, 165, 0, .18f, W_TRI, 1.5f),
                                         N(131, 131, .24f, .3f, W_TRI, 1.5f), N(0, 0, .24f, .05f, W_NOISE, .4f));
    else if (!strcmp(k, "bark")) /* latido: dois "wuf" (ruído curto + pulso caindo de tom + grave) */
        PLAY(N(0, 0, 0, .05f, W_NOISE, .9f), N(620, 230, 0, .13f, W_P50, 1.1f), N(180, 110, 0, .14f, W_TRI, 1.6f),
             N(0, 0, .2f, .05f, W_NOISE, .8f), N(560, 200, .2f, .16f, W_P50, 1), N(165, 100, .2f, .17f, W_TRI, 1.6f));
    else PLAY(N(988, 988, 0, .05f, W_P25, 1), N(1319, 1319, .05f, .12f, W_P25, 1));
}

void sfx_letter(int i) {
    static const float scale[12] = {C5, D5, E5, G5, A5, C6, 1175, E6, G6, 1760, 2093, 2349};
    float f = scale[((i % 12) + 12) % 12];
    Note n = {f, f, 0, .05f, W_P12, .55f};
    play(&n, 1, 0);
}

void sfx_mos(int k) {
    static const float MOS[8] = {262, 330, 392, 440, 523, 587, 659, 784};
    if (k >= 7) {
        PLAY(N(784, 784, 0, .05f, W_P25, 1), N(1175, 1175, .05f, .09f, W_P25, 1));
        return;
    }
    Note n = {MOS[k < 0 ? 0 : k], MOS[k < 0 ? 0 : k], 0, .045f, W_P25, .9f};
    play(&n, 1, 0);
}
