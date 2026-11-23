#include "audio.h"

#include <psp2/audioout.h>
#include <psp2/rtc.h>
#include <psp2/io/fcntl.h>
#include <psp2/kernel/threadmgr.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define RATE 48000
#define GRAIN 1024
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

/* Background OST modes: 0=Automatic, 1=Original (built-in synth),
 * 2=Morning Pixels, 3=Afternoon Arcade, 4=Night Shift.
 * PCM source: 24 kHz mono signed 16-bit, read in blocks from app0.
 * Resampling to the Vita's 48 kHz port uses 2x linear interpolation.
 * The original synthesized soundtrack is left untouched and selectable. */
#define OST_AUTO 0
#define OST_ORIGINAL 1
#define OST_MORNING 2
#define OST_AFTERNOON 3
#define OST_NIGHT 4
#define PCM_BLOCK 4096
#define PCM_PRELOAD_MAX (4 * 1024 * 1024)
/* The Vita shares its storage bus with massive ZIP extraction. Reading
 * 8 KB from app0 every 170 ms can block the realtime audio thread when
 * the install worker is extracting files to ux0. Keep all three small
 * PCM files in main RAM (~5 MiB total) BEFORE starting the audio thread.
 * Never perform filesystem reads from the BGM mixer if preloading succeeded. */
static int16_t *ost_pcm[5];
static unsigned ost_pcm_samples[5];
static const int16_t *pcm_memory;
static unsigned pcm_memory_samples, pcm_memory_pos;
#define OST_FADE_SAMPLES (RATE / 6)
static volatile int ost_mode = OST_AUTO;
static volatile int ost_desired = OST_NIGHT;
static volatile int ost_restart;
static int ost_hour = -1;
static int active_track = -1;       /* owns PCM state: audio thread only */
static int missing_track = -1;      /* avoid rapid reopening on failure */
static int fade_phase = 0;          /* 0=stable, 1=out, 2=in */
static float music_gain = 0;
static SceUID pcm_fd = -1;
static int16_t pcm_block[PCM_BLOCK];
static int pcm_used, pcm_at;
static int16_t pcm_a, pcm_b;
static int pcm_half;
static void ost_preload_pcm(void) {
    static const char *paths[5] = {
        NULL, NULL, "app0:data/music/morning.pcm",
        "app0:data/music/afternoon.pcm", "app0:data/music/night.pcm"
    };
    for (int track=OST_MORNING; track<=OST_NIGHT; track++) {
        SceUID fd=sceIoOpen(paths[track],SCE_O_RDONLY,0);
        if (fd<0) continue;
        SceOff bytes=sceIoLseek(fd,0,SCE_SEEK_END);
        sceIoLseek(fd,0,SCE_SEEK_SET);
        if (bytes<=0 || bytes>PCM_PRELOAD_MAX || (bytes&1)) {
            sceIoClose(fd);
            continue;
        }
        int16_t *data=malloc((size_t)bytes);
        if (!data) { sceIoClose(fd); continue; }
        unsigned offset=0;
        while (offset<(unsigned)bytes) {
            int rd=sceIoRead(fd,(char*)data+offset,
                              ((unsigned)bytes-offset)>32768 ? 32768 : (unsigned)bytes-offset);
            if (rd<=0) break;
            offset+=(unsigned)rd;
        }
        sceIoClose(fd);
        if (offset!=(unsigned)bytes) { free(data); continue; }
        ost_pcm[track]=data;
        ost_pcm_samples[track]=(unsigned)bytes/sizeof(int16_t);
    }
}


static int pcm_next(void) {
    if (pcm_memory && pcm_memory_samples) {
        /* Exact sample count, no storage access and no gap at loop boundary. */
        if (pcm_memory_pos>=pcm_memory_samples) pcm_memory_pos=0;
        return pcm_memory[pcm_memory_pos++];
    }
    if (pcm_fd < 0) return 0;
    if (pcm_at >= pcm_used) {
        int rd = sceIoRead(pcm_fd, pcm_block, sizeof(pcm_block));
        if (rd <= 0) {
            sceIoLseek(pcm_fd, 0, SCE_SEEK_SET);
            rd = sceIoRead(pcm_fd, pcm_block, sizeof(pcm_block));
        }
        if (rd <= 0) return 0;
        pcm_used = rd / (int)sizeof(pcm_block[0]);
        pcm_at = 0;
    }
    return pcm_block[pcm_at++];
}
static float pcm_value(void) {
    if (!pcm_half) {
        pcm_a = pcm_b;
        pcm_b = (int16_t)pcm_next();
        pcm_half = 1;
        return pcm_a / 32768.f;
    }
    pcm_half = 0;
    return ((int)pcm_a + (int)pcm_b) / 65536.f;
}
static void pcm_close(void) {
    if (pcm_fd >= 0) sceIoClose(pcm_fd);
    pcm_fd = -1;
    pcm_memory = NULL;
    pcm_memory_samples = pcm_memory_pos = 0;
    pcm_used = pcm_at = pcm_half = 0;
    pcm_a = pcm_b = 0;
}
static void switch_track(int next) {
    static const char *paths[5] = {
        NULL, NULL, "app0:data/music/morning.pcm",
        "app0:data/music/afternoon.pcm", "app0:data/music/night.pcm"};
    pcm_close();
    for (int i = 0; i < M_VOICES; i++) mvoices[i].state = 0;
    mpos = 0;
    active_track = next;
    if (next >= OST_MORNING && next <= OST_NIGHT) {
        if (ost_pcm[next] && ost_pcm_samples[next]) {
            pcm_memory = ost_pcm[next];
            pcm_memory_samples = ost_pcm_samples[next];
            pcm_memory_pos = 0;
            missing_track = -1;
            pcm_b = (int16_t)pcm_next();
        } else {
            /* Low-memory fallback: preserve playback using the old streaming
             * reader rather than failing to start music. */
            pcm_fd = sceIoOpen(paths[next], SCE_O_RDONLY, 0);
            if (pcm_fd < 0) {
                missing_track = next;
                active_track = OST_ORIGINAL;
            } else {
                missing_track = -1;
                pcm_used = pcm_at = 0;
                pcm_b = (int16_t)pcm_next();
            }
        }
    }
}
int audio_ost_mode(void) { return ost_mode; }
void audio_set_ost_mode(int mode) {
    if (mode < 0 || mode > OST_NIGHT) mode = OST_AUTO;
    ost_mode = mode;
    int wanted = mode == OST_AUTO
        ? (ost_hour >= 6 && ost_hour < 12 ? OST_MORNING
           : ost_hour >= 12 && ost_hour < 18 ? OST_AFTERNOON : OST_NIGHT)
        : mode;
    ost_desired = wanted;
    __sync_synchronize();
}
void audio_ost_refresh_clock(void) {
    SceDateTime t;
    if (sceRtcGetCurrentClockLocalTime(&t) >= 0) {
        if (ost_hour != (int)t.hour) {
            ost_hour = t.hour;
            if (ost_mode == OST_AUTO) audio_set_ost_mode(OST_AUTO);
        }
    }
}
static void ost_mix_begin_buffer(void) {
    int wanted = music_on ? ost_desired : 0;
    if (ost_restart) { ost_restart = 0; active_track = -1; music_gain = 0; }
    if (wanted == missing_track) wanted = OST_ORIGINAL;
    if (wanted && active_track != wanted && fade_phase != 1) fade_phase = 1;
}
static float ost_gain_step(void) {
    if (fade_phase == 1) {
        music_gain -= 1.f / OST_FADE_SAMPLES;
        if (music_gain <= 0.f) {
            music_gain = 0.f;
            switch_track(ost_desired == missing_track ? OST_ORIGINAL : ost_desired);
            fade_phase = 2;
        }
    } else if (fade_phase == 2) {
        music_gain += 1.f / OST_FADE_SAMPLES;
        if (music_gain >= 1.f) { music_gain = 1.f; fade_phase = 0; }
    }
    return music_gain;
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
        if (music_screen_ready) ost_mix_begin_buffer();
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
            float music_mix = 0;
            if (music_on && music_screen_ready) {
                if (active_track == OST_ORIGINAL) {
                    if (mpos % M_STEP == 0) music_step((int)(mpos / M_STEP));
                    if (++mpos >= (unsigned)(M_STEPS * M_STEP)) mpos = 0;
                }
                for (int k = 0; k < M_VOICES; k++) {
                    Voice *v = &mvoices[k];
                    if (v->state != 2) continue;
                    music_mix += osc(v) * .55f;
                    if (++v->t >= v->len) v->state = 0;
                }
                if (active_track >= OST_MORNING && active_track <= OST_NIGHT)
                    music_mix += pcm_value() * 2.1f;
                mix += music_mix * ost_gain_step();
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
    if (on) ost_restart = 1;
}
int audio_music(void) { return music_on; }

/* Chamar somente após vita2d_swap_buffers() apresentar a tela inicial.
 * A configuração de música permanece intacta; o sequenciador parte do início. */
void audio_start_music_after_first_frame(void) {
    if (music_screen_ready) return;
    mpos = 0;
    ost_restart = 1;
    __sync_synchronize();
    music_screen_ready = 1;
}

void audio_init(void) {
    music_build();
    ost_preload_pcm(); /* ~5 MB loaded once, not during unzip/write bursts */
    audio_ost_refresh_clock();
    audio_set_ost_mode(OST_AUTO);
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
    /* New UI cues use the original 8-bit pulse/triangle palette. */
    if (!strcmp(k, "download")) PLAY(N(330, 440, 0, .065f, W_TRI, .8f), N(659, 659, .045f, .055f, W_P12, .42f), N(988, 1175, .12f, .09f, W_P25, .56f));
    else if (!strcmp(k, "cancel")) PLAY(N(659, 392, 0, .075f, W_P25, .55f), N(330, 220, .06f, .13f, W_TRI, .65f), N(0, 0, .03f, .04f, W_NOISE, .17f));
    else if (!strcmp(k, "complete")) PLAY(N(523, 523, 0, .08f, W_P25, .57f), N(659, 659, .075f, .08f, W_P25, .60f), N(784, 784, .15f, .08f, W_P25, .6f), N(1047, 1047, .23f, .16f, W_TRI, .9f));
    else if (!strcmp(k, "notice")) PLAY(N(392, 392, 0, .075f, W_TRI, .7f), N(587, 587, .085f, .11f, W_P12, .52f), N(494, 494, .19f, .095f, W_TRI, .5f));
    else if (!strcmp(k, "updates")) PLAY(N(440, 440, 0, .065f, W_P12, .44f), N(523, 523, .07f, .07f, W_P12, .44f), N(659, 659, .14f, .12f, W_TRI, .7f));
    else if (!strcmp(k, "hover")) PLAY(N(1760, 1760, 0, .018f, W_P12, .35f));
    else if (!strcmp(k, "open")) PLAY(N(988, 988, 0, .05f, W_P25, 1), N(1319, 1319, .05f, .12f, W_P25, 1));
    else if (!strcmp(k, "back")) PLAY(N(784, 523, 0, .08f, W_P50, .9f), N(392, 262, .07f, .1f, W_P50, .9f));
    /* Falling 8-bit logo, built from the app's own pulse/triangle/noise synth.
       No decoded video or wav and no extra mixer thread. */
    else if (!strcmp(k, "logo_fall"))
        PLAY(N(960, 195, 0, .27f, W_P12, .36f), N(0, 0, .08f, .19f, W_NOISE, .12f));
    else if (!strcmp(k, "logo_hit3"))
        PLAY(N(220, 78, 0, .23f, W_TRI, 1.6f), N(0, 0, 0, .09f, W_NOISE, .70f), N(330, 220, 0, .07f, W_P25, .85f));
    else if (!strcmp(k, "logo_hit2"))
        PLAY(N(260, 92, 0, .23f, W_TRI, 1.6f), N(0, 0, 0, .09f, W_NOISE, .75f), N(392, 260, 0, .07f, W_P25, .85f));
    else if (!strcmp(k, "logo_hit1"))
        PLAY(N(310, 104, 0, .28f, W_TRI, 1.75f), N(0, 0, 0, .11f, W_NOISE, .80f), N(494, 310, 0, .08f, W_P25, .95f));
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
