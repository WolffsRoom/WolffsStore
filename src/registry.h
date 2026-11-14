#ifndef WS_REGISTRY_H
#define WS_REGISTRY_H

/* O que o Wolff's Store já instalou/baixou, salvo em ux0:data/WolffsStore/installed.txt.
 * Serve para dizer se cada download está instalado, atualizado ou desatualizado. */

#define MAX_REG 128

typedef struct {
    char slug[48];
    int dl;              /* índice do download no post */
    int kind;            /* DL_VPK, DL_TRANS, DL_PLUGIN, DL_FILE */
    char version[24];    /* versão instalada (a do store.json na hora) */
    char name[96];
    char path[160];      /* onde ficou: ux0:app/TID, ux0:rePatch/TID, ux0:Downloads/arquivo */
    long long when;      /* segundos desde 2000, só para ordenar */
} RegEntry;

void reg_load(void);
void reg_set(const char *slug, int dl, int kind, const char *version, const char *name, const char *path);
RegEntry *reg_find(const char *slug, int dl);
int reg_count(void);
RegEntry *reg_get(int i);

#endif
