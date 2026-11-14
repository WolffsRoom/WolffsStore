#ifndef WS_FSUTIL_H
#define WS_FSUTIL_H

#define WS_DATA   "ux0:data/WolffsStore"
#define WS_CACHE  WS_DATA "/cache"
#define WS_DL     WS_DATA "/dl"
#define WS_LOGS   WS_DATA "/logs"

/* registro com data/hora em ux0:data/WolffsStore/logs/wolffsstore.log (passou de 256 KB vira .old) */
void ws_log(const char *fmt, ...);

int fs_exists(const char *path);
int fs_is_dir(const char *path);
void fs_mkdir_p(const char *path);
void fs_rm_rf(const char *path);
char *fs_read(const char *path, int *size);          /* malloc, termina em \0 */
int fs_write(const char *path, const void *buf, int size);
int fs_copy(const char *src, const char *dst);
long long fs_free_space(const char *dev);            /* "ux0:" -> bytes livres, <0 se falhar */
int fs_grep_ci(const char *path, const char *needle); /* 1 se o arquivo contém o texto (sem diferenciar maiúsculas) */

/* param.sfo */
int sfo_get(const char *sfo_path, const char *key, char *out, int out_size);

/* "1.2.10" > "1.2.9" */
int version_cmp(const char *a, const char *b);

/* UTF-8 (só caracteres Latin-1, como o store.json gera) -> Latin-1 */
void utf8_to_latin1(const char *in, char *out, int n);
char *dup_latin1(const char *utf8);

#endif
