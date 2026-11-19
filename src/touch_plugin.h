#ifndef WS_TOUCH_PLUGIN_H
#define WS_TOUCH_PLUGIN_H
#include "net.h"
int touch_plugin_install(const char *archive, NetProgress *pr);
const char *touch_plugin_error(void);
#endif
