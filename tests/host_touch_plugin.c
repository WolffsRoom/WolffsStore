// Host-side functional regression harness for the real src/touch_plugin.c.
// No PlayStation Vita APIs, filesystem or real TaiHEN configuration are touched.
#define _XOPEN_SOURCE 700
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../src/touch_plugin.h"
#include "../src/fsutil.h"
#include "../src/install.h"
#include "../src/vendor/sha1.h"
#include "psp2/io/fcntl.h"
#include "psp2/io/stat.h"

static char root[PATH_MAX];
static int fail_cfg_write = 0;
static int fail_cfg_copy_once = 0;
static int fail_backup_once = 0;
static unsigned tests_passed = 0;
static const char *const config_ux="ux0:tai/config.txt";
static const char *const config_ur="ur0:tai/config.txt";
static const char *const plugin_ur="ur0:tai/DDTouchUI.suprx";

static const char *host_path(const char *vita) {
    static char names[8][PATH_MAX];
    static unsigned i = 0;
    char *out=names[(i++)%8];
    const size_t a=strlen(root),b=strlen(vita);
    if(a+1+b>=PATH_MAX)abort();
    memcpy(out,root,a);out[a]='/';memcpy(out+a+1,vita,b+1);
    return out;
}
static void mkparents(const char *full) {
    char path[PATH_MAX];snprintf(path,sizeof(path),"%s",full);
    for(char *p=path+1;*p;p++) {
        if(*p!='/') continue;
        *p=0;
        if(mkdir(path,0700)<0&&errno!=EEXIST){perror("mkdir");exit(2);}
        *p='/';
    }
}
static int write_bytes(const char *full,const void *data,size_t n) {
    mkparents(full);
    FILE *f=fopen(full,"wb"); if(!f)return -1;
    size_t w=fwrite(data,1,n,f);int rc=fclose(f);
    return w==n&&rc==0?0:-1;
}
static int read_bytes(const char *full,char **data) {
    *data=NULL;FILE *f=fopen(full,"rb");if(!f)return -1;
    if(fseek(f,0,SEEK_END)<0){fclose(f);return -1;}
    long n=ftell(f);if(n<0||fseek(f,0,SEEK_SET)<0){fclose(f);return -1;}
    char *buf=malloc((size_t)n+1);if(!buf){fclose(f);return -1;}
    if(fread(buf,1,(size_t)n,f)!=(size_t)n){free(buf);fclose(f);return -1;}
    fclose(f);buf[n]=0;*data=buf;return (int)n;
}
static int exists_full(const char *path) {return access(path,F_OK)==0;}
int fs_exists(const char *path){return exists_full(host_path(path));}
int fs_is_dir(const char *path){struct stat st;return stat(host_path(path),&st)==0&&S_ISDIR(st.st_mode);}
void fs_mkdir_p(const char *path){char tmp[PATH_MAX];snprintf(tmp,sizeof(tmp),"%s/",host_path(path));mkparents(tmp);}
static int rm_path(const char *f,const struct stat *st,int type,struct FTW *ftw) {
    (void)st;(void)type;(void)ftw;return remove(f);
}
void fs_rm_rf(const char *path){const char *full=host_path(path);if(exists_full(full))nftw(full,rm_path,16,FTW_DEPTH|FTW_PHYS);}
char *fs_read(const char *path,int *n){
    char *ret=NULL;int got=read_bytes(host_path(path),&ret);
    if(n)*n=got>=0?got:0;
    return ret;
}
int fs_write(const char *path,const void *data,int n){
    if(fail_cfg_write&&strstr(path,"config.txt.next")){
        fail_cfg_write=0;return -1;
    }
    return write_bytes(host_path(path),data,(size_t)n);
}
int fs_copy(const char *src,const char *dst) {
    if(fail_backup_once&&strstr(dst,"config.txt.before")){
        fail_backup_once=0;return -1;
    }
    if(fail_cfg_copy_once&&strstr(src,"config.txt.next")&&strstr(dst,"config.txt")){
        fail_cfg_copy_once=0;return -1;
    }
    char *raw=NULL;int n=read_bytes(host_path(src),&raw);
    if(n<0)return -1;
    int rc=write_bytes(host_path(dst),raw,(size_t)n);
    free(raw);return rc;
}
void ws_log(const char *fmt,...){(void)fmt;}
int sceIoOpen(const char *path,int flags,int mode){(void)flags;(void)mode;return open(host_path(path),O_RDONLY);}
int sceIoRead(int fd,void *buffer,int n){return (int)read(fd,buffer,(size_t)n);}
int sceIoClose(int fd){return close(fd);}
int sceIoRemove(const char *path){return remove(host_path(path));}
int sceIoRename(const char *old,const char *newname){
    // Vita commonly cannot rename over an existing config.txt. Exercise fallback.
    if(strstr(old,"config.txt.next")&&fs_exists(newname))return -1;
    char target[PATH_MAX],source[PATH_MAX];
    snprintf(target,sizeof(target),"%s",host_path(newname));
    snprintf(source,sizeof(source),"%s",host_path(old));
    mkparents(target);return rename(source,target);
}
int sceIoGetstat(const char *path,SceIoStat *out){
    struct stat st;
    if(stat(host_path(path),&st)<0)return -1;
    out->st_size=(long long)st.st_size;
    return 0;
}
// Stub the ZIP extractor only. The real installer is compiled unchanged,
// and its file-size, staging, config, backups, commit and rollback all run.
int inst_extract(const char *archive,const char *dest,const char **strip,int nstrip,NetProgress *pr){
    (void)archive;(void)strip;(void)nstrip;(void)pr;
    static const char *names[7]={
        "DDTouchUI.suprx","UI.rgba","Selector.rgba","On.rgba",
        "Off.rgba","Mod_on.rgba","Mod_off.rgba"};
    static const int sizes[7]={28771,882360,41832,14204,14204,145500,145500};
    for(int i=0;i<7;i++){
        char dst[300];snprintf(dst,sizeof(dst),"%s/%s",dest,names[i]);
        char *data=malloc((size_t)sizes[i]);if(!data)return -1;
        memset(data,65+i,(size_t)sizes[i]);
        int r=fs_write(dst,data,sizes[i]);free(data);
        if(r<0)return -1;
    }
    char cfgpath[300];snprintf(cfgpath,sizeof(cfgpath),"%s/mode.ini",dest);
    return fs_write(cfgpath,"touch=1\n",8);
}
// The archive hash is deliberately mocked to pass for a test fixture.
// This test does NOT validate SHA-1 security or the real ZIP package.
void sha1_init(SHA1_CTX *ctx){memset(ctx,0,sizeof(*ctx));}
void sha1_update(SHA1_CTX *ctx,const BYTE *bytes,size_t n){(void)ctx;(void)bytes;(void)n;}
void sha1_final(SHA1_CTX *ctx,BYTE digest[]){
    (void)ctx;
    static const BYTE expected[20]={
        0xeb,0xa9,0x2e,0x73,0x6d,0x51,0xd4,0xa2,0x05,0xbf,
        0x86,0xcc,0x29,0xf2,0x28,0x4e,0xff,0xbf,0x31,0x15};
    memcpy(digest,expected,20);
}
static void file(const char *path,const char *value){assert(fs_write(path,value,(int)strlen(value))==0);}
static int same(const char *path,const char *original){
    int n=0;char *s=fs_read(path,&n);
    int eq=s && n==(int)strlen(original) && memcmp(s,original,(size_t)n)==0;
    free(s);return eq;
}
static int contains(const char *path,const char *word){
    char *s=fs_read(path,NULL);
    int yes=s&&strstr(s,word)!=NULL;
    free(s);return yes;
}
static void reset(void){
    char tmp[PATH_MAX];snprintf(tmp,sizeof(tmp),"%s",root);
    nftw(tmp,rm_path,16,FTW_DEPTH|FTW_PHYS);
    assert(mkdir(root,0700)==0);
    fail_cfg_write=fail_cfg_copy_once=fail_backup_once=0;
    file("ux0:app/PCSE00919/sce_sys/param.sfo","game");
    file("ux0:data/package.zip","zip");
}
static void check(int ok,const char *label){
    if(!ok){fprintf(stderr,"FAIL: %s\n",label);exit(1);}
    tests_passed++;printf("PASS: %s\n",label);
}
int main(void){
    snprintf(root,sizeof(root),"/tmp/wolffs_touch_host_%d",(int)getpid());
    assert(mkdir(root,0700)==0);
    const char *ux_original="*main\nur0:tai/abc.suprx\n*PCSE00919\nur0:tai/other.suprx\n*other\nur0:tai/xyz.suprx\n";
    const char *ur_original="*main\nur0:tai/unrelated.suprx\n";
    // Both configs exist; active ux0 must be modified, ur0 preserved.
    reset();file(config_ux,ux_original);file(config_ur,ur_original);
    check(touch_plugin_install("ux0:data/package.zip",NULL)==0,"ux0+ur0 installation returns success");
    check(same(config_ur,ur_original),"inactive ur0 config untouched");
    check(contains(config_ux,"*PCSE00919\nur0:tai/other.suprx\nur0:tai/DDTouchUI.suprx\n"),
          "active ux0 config updated in the correct title section");
    check(same("ux0:data/WolffsStore/plugin-backups/DDTouchUI/config.txt.before",ux_original),
          "backup contains original ACTIVE ux0 config");
    check(fs_exists(plugin_ur),"plugin binary installed in ur0");
    // Only ur0 exists; fallback must select it.
    reset();file(config_ur,ur_original);
    check(touch_plugin_install("ux0:data/package.zip",NULL)==0,"ur0-only installation returns success");
    check(contains(config_ur,"*PCSE00919\nur0:tai/DDTouchUI.suprx\n"),
          "ur0 fallback updated correctly");
    check(!fs_exists(config_ux),"ur0 fallback does not create ux0 config");
    check(same("ux0:data/WolffsStore/plugin-backups/DDTouchUI/config.txt.before",ur_original),
          "ur0 backup contains original");
    // Failure after staged plugin write: rollback original active config,
    // inactive config, old plugin and mode.ini state.
    reset();file(config_ux,ux_original);file(config_ur,ur_original);
    file(plugin_ur,"previous-plugin");
    fail_cfg_write=1;
    check(touch_plugin_install("ux0:data/package.zip",NULL)<0,"commit write failure detected");
    check(same(config_ux,ux_original)&&same(config_ur,ur_original),
          "config rollback preserves active and inactive text");
    check(same(plugin_ur,"previous-plugin"),"previous installed plugin restored");
    check(!fs_exists("ux0:data/DDTouchUI/mode.ini"),"new mode.ini removed on rollback");
    check(!fs_exists("ux0:tai/config.txt.next"),"active config staging removed on rollback");
    // Rename fallback also needs to be rollback safe after failed copy.
    reset();file(config_ur,ur_original);
    fail_cfg_copy_once=1;
    check(touch_plugin_install("ux0:data/package.zip",NULL)<0,"rename fallback copy failure detected");
    check(same(config_ur,ur_original),"fallback restores original ur0 config");
    check(!fs_exists(plugin_ur),"failed install removes newly created plugin");
    check(!fs_exists("ur0:tai/config.txt.next"),"staged ur0 config removed on rollback");
    // Backup error must abort before writing the installed plugin/config.
    reset();file(config_ux,ux_original);file(config_ur,ur_original);
    fail_backup_once=1;
    check(touch_plugin_install("ux0:data/package.zip",NULL)<0,"backup failure detected");
    check(same(config_ux,ux_original)&&same(config_ur,ur_original)&&!fs_exists(plugin_ur),
          "backup failure leaves both configs and plugin unmodified");
    // Missing configs must never write silently to either partition.
    reset();
    check(touch_plugin_install("ux0:data/package.zip",NULL)<0,"missing config rejected");
    check(!fs_exists(config_ux)&&!fs_exists(config_ur)&&!fs_exists(plugin_ur),
          "missing config creates no target state");
    nftw(root,rm_path,16,FTW_DEPTH|FTW_PHYS);
    printf("SUCCESS: %u host assertions passed (real touch_plugin.c implementation)\n",tests_passed);
    return 0;
}
