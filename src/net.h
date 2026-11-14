#ifndef WS_NET_H
#define WS_NET_H

/* Rede via libcurl + OpenSSL (o SSL nativo do Vita não fecha TLS com o GitHub);
 * baseado no http.c do VitaSeed. */

typedef struct {
    volatile long long done, total;  /* bytes */
    volatile float speed;            /* bytes/s */
    volatile int cancel;             /* 1 = abortar */
} NetProgress;

enum { NET_OK = 0, NET_ERR = -1, NET_CANCELED = -2, NET_HTTP = -3, NET_DISK = -4, NET_OFFLINE = -5 };

void net_init(void);
int net_online(int wait_ms);                       /* 1 se conectado */
char *net_fetch(const char *url, int *size);      /* corpo inteiro (malloc, termina em \0) ou NULL */
int net_download(const char *url, const char *path, NetProgress *pr); /* NET_* */
const char *net_error(void);                      /* texto do último erro */

#endif
