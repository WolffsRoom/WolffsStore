/* Vita polls: background HTTPS, pseudonymous device deduplication.
 * The raw SceKernelOpenPsId is never transmitted nor stored.
 */
#include "polls.h"
#include "net.h"
#include "fsutil.h"
#include "json.h"

#include <psp2/kernel/openpsid.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/rtc.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define VOTES_DIR WS_DATA "/votes"
static char api[256];
static PollState current = {.selected=-1};
static volatile int running;
static int mode; /* 1 status, 2 submit, 3 results */
static int choice_request;

static int safe_id(const char *id) {
    if (!id || !id[0] || strlen(id) >= 48) return 0;
    for (const char *c=id; *c; c++)
        if (!(*c >= 'a' && *c <= 'z') && !(*c >= '0' && *c <= '9') && *c != '-')
            return 0;
    return 1;
}
static void marker_path(const char *id, char *path, size_t cap) {
    snprintf(path, cap, VOTES_DIR "/%s.txt", id);
}
int poll_local_answered(const char *id) {
    if (!safe_id(id)) return 0;
    char path[160];
    marker_path(id,path,sizeof(path));
    return fs_exists(path);
}
static void mark_answered(int selection) {
    if (!safe_id(current.id)) return;
    fs_mkdir_p(VOTES_DIR);
    char path[160], value[24];
    marker_path(current.id,path,sizeof(path));
    snprintf(value,sizeof(value),"%d",selection);
    fs_write(path,value,(int)strlen(value));
}

void poll_set_api(const char *value) {
    if (!value || strncmp(value,"https://",8) || strlen(value)>=sizeof(api) ||
        strchr(value,'?') || strchr(value,'#')) {
        api[0]=0;
        return;
    }
    snprintf(api,sizeof(api),"%s",value);
}
int poll_api_ready(void){return api[0]!=0;}

static int device_pseudonym(char out[65]) {
    SceKernelOpenPsId raw;
    if (sceKernelGetOpenPsId(&raw) < 0) return 0;
    /* A stable non-secret hash means reinstalling the Store preserves identity.
     * The server adds a secret per-poll HMAC for the stored vote key. */
    static const char domain[]="WolffsRoom/VitaPolls/1";
    unsigned char material[sizeof(domain)-1+16];
    memcpy(material,domain,sizeof(domain)-1);
    memcpy(material+sizeof(domain)-1,raw.id,16);
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256(material,sizeof(material),digest);
    memset(material,0,sizeof(material));
    memset(&raw,0,sizeof(raw));
    for (int i=0;i<SHA256_DIGEST_LENGTH;i++)
        snprintf(out+2*i,3,"%02x",digest[i]);
    out[64]=0;
    return 1;
}

int poll_local_ended(const char *ends) {
    if (!ends || strlen(ends)<16) return 0;
    SceDateTime now;
    if (sceRtcGetCurrentClockLocalTime(&now)<0) return 0;
    char time_text[32];
    snprintf(time_text,sizeof(time_text),"%04d-%02d-%02dT%02d:%02d",
             now.year,now.month,now.day,now.hour,now.minute);
    return strcmp(time_text,ends)>=0;
}
int poll_days_remaining(const char *ends) {
    if (!ends || strlen(ends)<16) return 0;
    int y=0,m=0,d=0,hh=0,mm=0;
    if (sscanf(ends,"%d-%d-%dT%d:%d",&y,&m,&d,&hh,&mm)!=5)return 0;
    SceDateTime now;
    if (sceRtcGetCurrentClockLocalTime(&now)<0)return 0;
    struct tm target={0},today={0};
    target.tm_year=y-1900;target.tm_mon=m-1;target.tm_mday=d;
    target.tm_hour=hh;target.tm_min=mm;target.tm_isdst=-1;
    today.tm_year=now.year-1900;today.tm_mon=now.month-1;today.tm_mday=now.day;
    today.tm_hour=now.hour;today.tm_min=now.minute;today.tm_isdst=-1;
    double seconds=difftime(mktime(&target),mktime(&today));
    int days=(int)(seconds/86400.0)+1;
    return days<0?0:days;
}

static int worker(SceSize arg, void *param) {
    (void)arg; (void)param;
    char url[440], request[512], answer[8192], hash[65];
    if (!device_pseudonym(hash)) {
        current.error=2;
        current.busy=0;
        current.version++;
        running=0;
        return sceKernelExitDeleteThread(0);
    }
    if (mode==3) {
        snprintf(url,sizeof(url),"%s/api/v1/results/%s",api,current.id);
        int http=net_json_verified(url,NULL,answer,sizeof(answer));
        if (http==200) {
            JsonValue *root=json_parse(answer);
            const JsonValue *counts=json_object_get(root,"counts");
            int n=json_array_count(counts);
            if (n==current.count) {
                current.total=(int)json_number(json_object_get(root,"total"),0);
                for (int i=0;i<n;i++)
                    current.votes[i]=(int)json_number(json_array_get(counts,i),0);
                current.results_ready=1;
            } else current.error=3;
            json_free(root);
        } else current.error=3;
    } else {
        const char *path=mode==2?"/api/v1/vote":"/api/v1/status";
        snprintf(url,sizeof(url),"%s%s",api,path);
        if (mode==2)
            snprintf(request,sizeof(request),
                "{\"poll\":\"%s\",\"device\":\"%s\",\"choice\":%d}",
                current.id,hash,choice_request);
        else
            snprintf(request,sizeof(request),
                "{\"poll\":\"%s\",\"device\":\"%s\"}",current.id,hash);
        int http=net_json_verified(url,request,answer,sizeof(answer));
        if (http==201 || http==409 || http==200) {
            JsonValue *root=json_parse(answer);
            int answered=json_bool(json_object_get(root,"answered"),0);
            if (answered) {
                current.answered=1;
                if (mode==2) {
                    current.selected=choice_request;
                    mark_answered(choice_request);
                } else {
                    int remote_choice=(int)json_number(json_object_get(root,"choice"),-1);
                    if (remote_choice>=0 && remote_choice<current.count)
                        current.selected=remote_choice;
                    if (!poll_local_answered(current.id))mark_answered(current.selected);
                }
            } else if(mode==2)current.error=4;
            json_free(root);
        } else current.error=http<0?1:4;
    }
    current.busy=0;
    current.version++;
    running=0;
    return sceKernelExitDeleteThread(0);
}
static int request_async(int request_mode,int choice) {
    if (!poll_api_ready() || running) return 0;
    mode=request_mode;
    choice_request=choice;
    current.busy=1;current.error=0;running=1;
    SceUID th=sceKernelCreateThread("ws_poll_worker",worker,
                                    0x10000100-16,0x10000,0,0,NULL);
    if (th<0 || sceKernelStartThread(th,0,NULL)<0) {
        if(th>=0)sceKernelDeleteThread(th);
        current.busy=0;running=0;current.error=1;
        return 0;
    }
    return 1;
}
void poll_open(const char *id,const char *ends,int count,int ended) {
    if (!safe_id(id) || count<2 || count>MAX_POLL_CHOICES || running) return;
    memset(&current,0,sizeof(current));
    snprintf(current.id,sizeof(current.id),"%s",id);
    snprintf(current.end_at,sizeof(current.end_at),"%s",ends?ends:"");
    current.count=count;
    current.selected=-1;
    current.answered=poll_local_answered(id);
    if (current.answered) {
        char path[160];marker_path(id,path,sizeof(path));
        char *txt=fs_read(path,NULL);
        if(txt){current.selected=atoi(txt);free(txt);}
    }
    current.version++;
    if (ended) request_async(3,0);
    else request_async(1,0);
}
int poll_vote(int choice) {
    if (choice<0 || choice>=current.count || current.answered || running ||
        poll_local_ended(current.end_at))return 0;
    return request_async(2,choice);
}
void poll_state(PollState *out) {if(out)memcpy(out,&current,sizeof(current));}
