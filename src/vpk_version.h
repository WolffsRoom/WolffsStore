#ifndef WS_VPK_VERSION_H
#define WS_VPK_VERSION_H

#include <stdio.h>
#include <string.h>
#include "fsutil.h"

/* A published VPK can have a stale APP_VER in param.sfo (Deltarune
 * v0.81 ships APP_VER=00.80). The Store only trusts the newer catalog
 * version after its own promoter successfully installed that exact app:
 * installed.txt must still match the same ux0:app/TITLE_ID.
 *
 * The SFO remains the source for installations made outside the Store,
 * mismatching TitleIDs, or versions newer than the Store's record.
 * The caller must additionally check that the registry kind is DL_VPK
 * and that the installed app actually has a readable param.sfo. */
static inline const char *ws_vpk_installed_version(
    const char *sfo_version, const char *recorded_version,
    const char *recorded_path, const char *installed_titleid) {
    const char *sfo = sfo_version ? sfo_version : "";
    if (!recorded_version || !recorded_version[0] ||
        !recorded_path || !installed_titleid ||
        strlen(installed_titleid) != 9) return sfo;
    char expected[32];
    snprintf(expected, sizeof(expected), "ux0:app/%s", installed_titleid);
    if (strcmp(recorded_path, expected) != 0) return sfo;
    if (sfo[0] && version_cmp(recorded_version, sfo) <= 0) return sfo;
    return recorded_version;
}

#endif
