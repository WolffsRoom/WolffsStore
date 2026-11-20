#ifndef MOCK_VITA_FCNTL_H
#define MOCK_VITA_FCNTL_H
typedef int SceUID;
#define SCE_O_RDONLY 0
int sceIoOpen(const char *, int, int);
int sceIoRead(int, void *, int);
int sceIoClose(int);
int sceIoRename(const char *, const char *);
int sceIoRemove(const char *);
#endif
