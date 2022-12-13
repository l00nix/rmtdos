#ifndef __RMTDOS_CLIENT_GLOBALS_H
#define __RMTDOS_CLIENT_GLOBALS_H

#include "client/hostlist.h"

extern int g_running;
extern int g_show_debug_window;

// Non-NULL if we're actively controlling a server.
extern struct RemoteHost *g_active_host;

#endif // __RMTDOS_CLIENT_GLOBALS_H
