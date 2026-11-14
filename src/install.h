#ifndef WS_INSTALL_H
#define WS_INSTALL_H

#include "net.h"

#define WS_PKG_TMP   "ux0:data/WolffsStore/pkg_tmp"  /* VPK de projetos sendo instalado */
#define WS_PKG_SELF  "ux0:data/WolffsStore/pkg"      /* atualização do próprio app (o updater instala) */
#define WS_PKG_UPD   "ux0:data/WolffsStore/updater_pkg"

/* Extrai um zip/VPK em dst. Se strip != NULL, tira esses prefixos do começo de cada caminho
 * (ex.: "rePatch/PCSE00584/"). pr->done/total contam arquivos. 0 = ok. */
int inst_extract(const char *zip, const char *dst, const char **strip, int nstrip, NetProgress *pr);
int inst_make_head_bin(const char *pkg_dir);
int inst_promote(const char *pkg_dir);
int inst_delete(const char *titleid);
/* VPK completo: extrai em WS_PKG_TMP, cria o head.bin e instala com o promoter. */
int inst_install_vpk(const char *vpk, NetProgress *pr, volatile int *phase);
const char *inst_error(void);
const char *inst_last_titleid(void); /* TITLE_ID do último VPK instalado */

#endif
