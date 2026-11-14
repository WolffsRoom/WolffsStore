#ifndef WS_AUDIO_H
#define WS_AUDIO_H

/* Sintetizador chiptune com os mesmos sons do site (pulso 12,5/25/50%,
 * triângulo, ruído e eco). Os nomes são os mesmos do template do site. */

void audio_init(void);
void audio_set_enabled(int on);   /* efeitos sonoros */
int audio_enabled(void);
void audio_set_music(int on);     /* música de fundo (chiptune em loop) */
int audio_music(void);
void sfx(const char *name);
void sfx_letter(int i);  /* notas das letras do logo (hovL) */
void sfx_mos(int k);     /* depixelização da capa (mos0..7) */

#endif
