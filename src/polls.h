#ifndef WS_POLLS_H
#define WS_POLLS_H
#define MAX_POLL_CHOICES 8
typedef struct {
    char id[48], end_at[32];
    int busy, answered, selected, results_ready, total, count;
    int votes[MAX_POLL_CHOICES];
    int error;
    unsigned version;
} PollState;
void poll_set_api(const char *api);
int poll_api_ready(void);
int poll_local_answered(const char *id);
void poll_open(const char *id, const char *ends_at, int noptions, int ended);
int poll_vote(int choice);
void poll_state(PollState *out);
int poll_days_remaining(const char *ends_at);
int poll_local_ended(const char *ends_at);
#endif
