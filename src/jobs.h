#ifndef WS_JOBS_H
#define WS_JOBS_H

#include "catalog.h"
#include "net.h"

/* Fila de downloads/instalações em segundo plano (estilo PKGj): um trabalho por vez,
 * a interface continua livre. */

enum { J_QUEUED, J_DOWNLOAD, J_EXTRACT, J_INSTALL, J_DONE, J_ERROR, J_CANCELED };
enum { JK_VPK, JK_TRANS, JK_PLUGIN, JK_FILE, JK_SELF, JK_DATA };

#define MAX_JOBS 24

typedef struct {
    int id, kind;
    char name[96], url[512], file[128], version[24];
    char slug[48];
    int dl;                 /* índice do download no post */
    char tids[8][10];
    int ntids;
    long long size_hint;
    int used;
    volatile int state;
    volatile int phase;     /* VPK: 0 extraindo, 1 instalando */
    NetProgress pr;
    char msg[192];          /* erro, ou onde ficou o arquivo (Latin-1) */
    int seen;               /* a interface já avisou do resultado */
} Job;

void jobs_init(void);
int jobs_add(const Post *p, int dl_index, const char *title); /* id ou -1 */
int jobs_add_self(const AppRelease *app);                  /* atualização do app */
int jobs_count(void);
Job *jobs_get(int i);                                      /* 0 = mais antigo */
Job *jobs_find(const char *slug, int dl_index);            /* trabalho mais recente desse download */
int jobs_active(void);                                     /* na fila ou rodando */
void jobs_cancel(int id);
void jobs_clear_finished(void);
unsigned jobs_version(void);                               /* muda quando algum estado muda */

#endif
