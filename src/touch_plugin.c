/*
 * TouchOnVita installer, PCSE00919 / release v0.59 only.
 * Modifies taiHEN only after explicit store confirmation. Each existing
 * configuration/plugin is backed up before the first write.
 */
#include "touch_plugin.h"
#include "fsutil.h"
#include "install.h"
#include "vendor/sha1.h"

#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DD_STAGE "ux0:data/WolffsStore/ddtouch_stage"
#define DD_BACKUP "ux0:data/WolffsStore/plugin-backups/DDTouchUI"
#define DD_DATA "ux0:data/DDTouchUI"
#define DD_PLUGIN "ur0:tai/DDTouchUI.suprx"
#define DD_CONFIG "ur0:tai/config.txt"

static char err[192];
const char *touch_plugin_error(void) { return err; }

static int bad(const char *msg) {
    snprintf(err, sizeof(err), "%s", msg);
    ws_log("touch plugin: %s", msg);
    return -1;
}

/* This allowlist prevents an altered store.json from injecting unrelated
 * user plugins or overwriting arbitrary taiHEN entries.
 * The v0.59 release ZIP has SHA1 eba92e736d51d4a205bf86cc29f2284effbf3115.
 * New releases must be reviewed and allowlisted in a Store update. */
static int approved_zip(const char *file) {
    static const uint8_t expected[20]={
        0xeb,0xa9,0x2e,0x73,0x6d,0x51,0xd4,0xa2,0x05,0xbf,
        0x86,0xcc,0x29,0xf2,0x28,0x4e,0xff,0xbf,0x31,0x15
    };
    SceUID fd=sceIoOpen(file,SCE_O_RDONLY,0);
    if(fd<0)return 0;
    SHA1_CTX ctx;
    sha1_init(&ctx);
    uint8_t buf[8192],digest[20];
    int n=0;
    while((n=sceIoRead(fd,buf,sizeof(buf)))>0)
        sha1_update(&ctx,buf,(size_t)n);
    sceIoClose(fd);
    if(n<0)return 0;
    sha1_final(&ctx,digest);
    return memcmp(expected,digest,sizeof(expected))==0;
}

/* Return a new taiHEN config retaining unrelated entries byte-for-byte. */
static char *config_with_plugin(const char *src,size_t len){
    const char *section="*PCSE00919";
    const char *line="ur0:tai/DDTouchUI.suprx";
    if(!src || len>65536)return NULL;
    char *dst=malloc(len+128);
    if(!dst)return NULL;
    size_t w=0;
    int inside=0,section_seen=0,entry_seen=0;
    for(size_t pos=0;pos<len;){
        size_t start=pos;
        while(pos<len && src[pos]!='\n')pos++;
        size_t end=pos;
        if(pos<len)pos++;
        size_t n=end-start;
        while(n && (src[start+n-1]=='\r'||src[start+n-1]==' '))n--;
        if(n && src[start]=='*'){
            if(inside && !entry_seen){
                size_t plen=strlen(line);
                memcpy(dst+w,line,plen);w+=plen;dst[w++]='\n';
                entry_seen=1;
            }
            inside=n==strlen(section) && !memcmp(src+start,section,n);
            if(inside)section_seen=1;
        }else if(inside && n==strlen(line) && !memcmp(src+start,line,n)){
            entry_seen=1;
        }
        size_t chunk=pos-start;
        memcpy(dst+w,src+start,chunk);w+=chunk;
    }
    if(!section_seen){
        if(w && dst[w-1]!='\n')dst[w++]='\n';
        size_t n=strlen(section);memcpy(dst+w,section,n);w+=n;dst[w++]='\n';
        n=strlen(line);memcpy(dst+w,line,n);w+=n;dst[w++]='\n';
    }else if(inside && !entry_seen){
        if(w && dst[w-1]!='\n')dst[w++]='\n';
        size_t n=strlen(line);memcpy(dst+w,line,n);w+=n;dst[w++]='\n';
    }
    dst[w]=0;
    return dst;
}

static int size_is(const char *path, int count){
    SceIoStat st;
    return sceIoGetstat(path,&st)>=0 && st.st_size==count;
}
static void revert_file(const char *target,const char *backup,int had){
    if(had)fs_copy(backup,target);
    else sceIoRemove(target);
}

int touch_plugin_install(const char *archive, NetProgress *pr){
    err[0]=0;
    if(!fs_exists("ux0:app/PCSE00919/sce_sys/param.sfo"))
        return bad("Darkest Dungeon PCSE00919 nao encontrado.");
    if(!approved_zip(archive))
        return bad("Pacote nao autorizado ou modificado.");
    fs_rm_rf(DD_STAGE);
    fs_mkdir_p(DD_STAGE);
    if(inst_extract(archive,DD_STAGE,NULL,0,pr)<0){
        fs_rm_rf(DD_STAGE);
        return bad("Falha ao extrair arquivo.");
    }

    static const char *names[7]={
        "DDTouchUI.suprx","UI.rgba","Selector.rgba","On.rgba",
        "Off.rgba","Mod_on.rgba","Mod_off.rgba"
    };
    static const int sizes[7]={
        28771,882360,41832,14204,14204,145500,145500
    };
    char path[256],backup[256];
    for(int i=0;i<7;i++){
        snprintf(path,sizeof(path),"%s/%s",DD_STAGE,names[i]);
        if(!size_is(path,sizes[i])){
            fs_rm_rf(DD_STAGE);
            return bad("Arquivos do plugin incompletos.");
        }
    }
    if(!fs_exists(DD_STAGE "/mode.ini")){
        fs_rm_rf(DD_STAGE);
        return bad("Configuracao inicial ausente.");
    }
    int oldsize=0;
    char *oldcfg=fs_read(DD_CONFIG,&oldsize);
    if(!oldcfg || oldsize<1 || oldsize>65536){
        free(oldcfg);fs_rm_rf(DD_STAGE);
        return bad("config.txt indisponivel. Nada alterado.");
    }
    char *cfg=config_with_plugin(oldcfg,(size_t)oldsize);
    free(oldcfg);
    if(!cfg){
        fs_rm_rf(DD_STAGE);
        return bad("Nao foi possivel preparar config.txt.");
    }

    fs_mkdir_p(DD_BACKUP);
    if(fs_copy(DD_CONFIG,DD_BACKUP "/config.txt.before")<0){
        free(cfg);fs_rm_rf(DD_STAGE);
        return bad("Backup do taiHEN falhou.");
    }
    int existed[7]={0};
    /* Validate and back up all original files BEFORE changing a single one. */
    for(int i=0;i<7;i++){
        snprintf(path,sizeof(path),"%s/%s",i?DD_DATA:"ur0:tai",names[i]);
        existed[i]=fs_exists(path);
        if(existed[i]){
            snprintf(backup,sizeof(backup),"%s/%s.bak",DD_BACKUP,names[i]);
            if(fs_copy(path,backup)<0){
                free(cfg);fs_rm_rf(DD_STAGE);
                return bad("Backup dos arquivos existentes falhou.");
            }
        }
    }
    fs_mkdir_p(DD_DATA);
    int hadmode=fs_exists(DD_DATA "/mode.ini");
    if(!hadmode && fs_copy(DD_STAGE "/mode.ini",DD_DATA "/mode.ini")<0){
        free(cfg);fs_rm_rf(DD_STAGE);
        return bad("Falha ao criar mode.ini.");
    }

    int changed=0,plugin_changed=0;
    for(int i=1;i<7;i++){
        char src[256];
        snprintf(src,sizeof(src),"%s/%s",DD_STAGE,names[i]);
        snprintf(path,sizeof(path),"%s/%s",DD_DATA,names[i]);
        changed=i;
        if(fs_copy(src,path)<0)goto rollback;
    }
    if(fs_copy(DD_STAGE "/DDTouchUI.suprx",DD_PLUGIN ".next")<0)goto rollback;
    if(!size_is(DD_PLUGIN ".next",sizes[0]))goto rollback;
    sceIoRemove(DD_PLUGIN);
    plugin_changed=1;
    if(sceIoRename(DD_PLUGIN ".next",DD_PLUGIN)<0)goto rollback;

    if(fs_write(DD_CONFIG ".next",cfg,(int)strlen(cfg))<0)goto rollback;
    /* Some Vita configurations do not allow replacing an open destination
       by rename; safely fall back to copy, with a verified backup above. */
    if(sceIoRename(DD_CONFIG ".next",DD_CONFIG)<0){
        if(fs_copy(DD_CONFIG ".next",DD_CONFIG)<0)goto rollback;
        sceIoRemove(DD_CONFIG ".next");
    }
    int result_size=0;
    char *result=fs_read(DD_CONFIG,&result_size);
    int correct=result && result_size==(int)strlen(cfg) &&
                !memcmp(result,cfg,(size_t)result_size);
    free(result);
    if(!correct)goto rollback;

    free(cfg);
    fs_rm_rf(DD_STAGE);
    ws_log("TouchOnVita 0.59 installed in ur0:tai and ux0:data");
    return 0;
rollback:
    for(int i=1;i<=changed;i++){
        snprintf(path,sizeof(path),"%s/%s",DD_DATA,names[i]);
        snprintf(backup,sizeof(backup),"%s/%s.bak",DD_BACKUP,names[i]);
        revert_file(path,backup,existed[i]);
    }
    if(plugin_changed){
        snprintf(backup,sizeof(backup),"%s/%s.bak",DD_BACKUP,names[0]);
        revert_file(DD_PLUGIN,backup,existed[0]);
    }
    if(!hadmode)sceIoRemove(DD_DATA "/mode.ini");
    fs_copy(DD_BACKUP "/config.txt.before",DD_CONFIG);
    sceIoRemove(DD_CONFIG ".next");
    sceIoRemove(DD_PLUGIN ".next");
    free(cfg);
    fs_rm_rf(DD_STAGE);
    return bad("Instalacao falhou; backup restaurado.");
}
