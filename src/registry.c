#include "registry.h"
#include "fsutil.h"

#include <psp2/io/fcntl.h>
#include <psp2/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REG_FILE WS_DATA "/installed.txt"

static RegEntry reg[MAX_REG];
static int nreg;

int reg_count(void) { return nreg; }
RegEntry *reg_get(int i) { return i >= 0 && i < nreg ? &reg[i] : NULL; }

RegEntry *reg_find(const char *slug, int dl) {
    for (int i = 0; i < nreg; i++)
        if (reg[i].dl == dl && !strcmp(reg[i].slug, slug)) return &reg[i];
    return NULL;
}

void reg_load(void) {
    nreg = 0;
    char *t = fs_read(REG_FILE, NULL);
    if (!t) return;
    char *save = NULL;
    for (char *line = strtok_r(t, "\n", &save); line && nreg < MAX_REG; line = strtok_r(NULL, "\n", &save)) {
        RegEntry *e = &reg[nreg];
        memset(e, 0, sizeof(*e));
        char *f[7] = {0};
        int n = 0;
        for (char *p = line; n < 7; n++) {
            f[n] = p;
            char *tab = strchr(p, '\t');
            if (!tab) {
                n++;
                break;
            }
            *tab = 0;
            p = tab + 1;
        }
        if (n < 7) continue;
        snprintf(e->slug, sizeof(e->slug), "%s", f[0]);
        e->dl = atoi(f[1]);
        e->kind = atoi(f[2]);
        snprintf(e->version, sizeof(e->version), "%s", f[3]);
        snprintf(e->name, sizeof(e->name), "%s", f[4]);
        snprintf(e->path, sizeof(e->path), "%s", f[5]);
        e->when = atoll(f[6]);
        nreg++;
    }
    free(t);
}

static void reg_save(void) {
    fs_mkdir_p(WS_DATA);
    FILE *f = fopen(REG_FILE ".tmp", "w");
    if (!f) return;
    for (int i = 0; i < nreg; i++)
        fprintf(f, "%s\t%d\t%d\t%s\t%s\t%s\t%lld\n", reg[i].slug, reg[i].dl, reg[i].kind, reg[i].version, reg[i].name,
                reg[i].path, reg[i].when);
    fclose(f);
    sceIoRemove(REG_FILE);
    sceIoRename(REG_FILE ".tmp", REG_FILE);
}

static long long now_secs(void) {
    SceDateTime t;
    if (sceRtcGetCurrentClockLocalTime(&t) < 0) return 0;
    return (((long long)(t.year - 2000) * 372 + (t.month - 1) * 31 + (t.day - 1)) * 24 + t.hour) * 3600 + t.minute * 60 + t.second;
}

void reg_set(const char *slug, int dl, int kind, const char *version, const char *name, const char *path) {
    RegEntry *e = reg_find(slug, dl);
    if (!e) {
        if (nreg >= MAX_REG) return;
        e = &reg[nreg++];
        memset(e, 0, sizeof(*e));
    }
    snprintf(e->slug, sizeof(e->slug), "%s", slug);
    e->dl = dl;
    e->kind = kind;
    snprintf(e->version, sizeof(e->version), "%s", version ? version : "");
    snprintf(e->name, sizeof(e->name), "%s", name ? name : "");
    snprintf(e->path, sizeof(e->path), "%s", path ? path : "");
    for (char *p = e->name; *p; p++)
        if (*p == '\t' || *p == '\n') *p = ' ';
    e->when = now_secs();
    reg_save();
}
