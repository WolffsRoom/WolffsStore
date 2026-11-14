#ifndef WS_VIDEO_H
#define WS_VIDEO_H

/* Reprodutor de vídeo dentro do app (sceAvPlayer).
 * O .mp4 (H.264 + AAC, até 960x544) é baixado para o cache e tocado em tela cheia. */

enum { VID_OFF, VID_LOADING, VID_PLAYING, VID_PAUSED, VID_ERROR };

/* começa: baixa (se ainda não estiver no cache) e toca */
void video_open(const char *url, const char *slug);
void video_close(void);
int video_state(void);
float video_load_progress(void);  /* 0..1 enquanto baixa */
const char *video_error(void);
void video_toggle_pause(void);
void video_seek(int seconds);      /* relativo */
/* chame a cada quadro: atualiza e desenha em tela cheia (960x544) */
void video_draw(void);
/* posição e duração em ms (0 se não souber) */
unsigned long long video_time_ms(void);

#endif
