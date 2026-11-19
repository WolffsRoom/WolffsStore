/* Host-buildable test of the installed VPK version policy. */
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int version_cmp(const char *a, const char *b) {
    int part = 0;
    while (*a || *b) {
        while (*a && !isdigit((unsigned char)*a)) a++;
        while (*b && !isdigit((unsigned char)*b)) b++;
        const char *as = a, *bs = b;
        long x = strtol(a, (char **)&a, 10);
        long y = strtol(b, (char **)&b, 10);
        if (part == 1) {
            if (a - as == 1) x *= 10;
            if (b - bs == 1) y *= 10;
        }
        if (x != y) return x < y ? -1 : 1;
        if (!*a && !*b) break;
        part++;
    }
    return 0;
}
#include "vpk_version.h"

int main(void) {
    /* Actual release Deltarune-v0.81.vpk has APP_VER=00.80. */
    const char *fixed = ws_vpk_installed_version(
        "00.80", "0.81", "ux0:app/DELT00001", "DELT00001");
    assert(strcmp(fixed, "0.81") == 0);
    assert(version_cmp(fixed, "0.82") < 0);
    assert(strcmp(ws_vpk_installed_version(
        "00.64", "0.64", "ux0:app/PTSAGE001", "PTSAGE001"), "00.64") == 0);
    assert(strcmp(ws_vpk_installed_version(
        "00.80", NULL, NULL, "DELT00001"), "00.80") == 0);
    assert(strcmp(ws_vpk_installed_version(
        "00.80", "0.81", "ux0:app/OTHER0001", "DELT00001"), "00.80") == 0);
    assert(strcmp(ws_vpk_installed_version(
        "00.80", "0.81", "ux0:rePatch/DELT00001", "DELT00001"), "00.80") == 0);
    assert(strcmp(ws_vpk_installed_version(
        "00.80", "0.81", "ux0:app/DELT00001", "SHORT"), "00.80") == 0);
    assert(strcmp(ws_vpk_installed_version(
        "00.85", "0.81", "ux0:app/DELT00001", "DELT00001"), "00.85") == 0);
    assert(strcmp(ws_vpk_installed_version(
        "01.00", "1.2", "ux0:app/EXAMP0001", "EXAMP0001"), "1.2") == 0);
    puts("9 VPK install-status assertions passed");
    return 0;
}
