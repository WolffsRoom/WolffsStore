#ifndef WS_SYNC_H
#define WS_SYNC_H

#include "catalog.h"

#define STORE_BASE "https://wolffsroom.github.io/WolffsRoomSite/"
#define STORE_JSON STORE_BASE "store-v105.json"
#define STORE_CACHE_JSON WS_CACHE "/store-v105.json"
#define RELEASE_API "https://api.github.com/repos/WolffsRoom/WolffsStore/releases/latest"
#define RELEASE_ASSET "WolffsStore.vpk"

enum { SYNC_IDLE, SYNC_RUNNING, SYNC_READY, SYNC_OFFLINE, SYNC_FAIL };

/* Sincroniza em segundo plano: baixa o store.json e as imagens que faltam no cache. */
void sync_start(void);
int sync_state(void);
/* progresso das imagens (0..1) enquanto roda */
float sync_progress(void);
/* Se terminou com dados novos, copia para dst (e app) e devolve o número de posts; senão -1. */
int sync_take(Post *dst, int max, AppRelease *app);
/* Lê o store.json do cache do cartão (abrir rápido / sem internet). */
int store_load_cache(Post *dst, int max, AppRelease *app);

#endif
