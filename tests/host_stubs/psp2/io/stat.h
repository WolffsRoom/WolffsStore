#ifndef MOCK_VITA_STAT_H
#define MOCK_VITA_STAT_H
typedef struct { long long st_size; } SceIoStat;
int sceIoGetstat(const char *, SceIoStat *);
#endif
