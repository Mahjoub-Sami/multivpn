#ifndef _PLUGIN__H
#define _PLUGIN__H

#include "globals.h"

int plugin_checkConfig(t_global_v); // Check if all options needed by plugin have been set

int plugin_build();
int plugin_start();
void plugin_stop();
int plugin_getTunTap();

int plugin_fillFDSET(fd_set *);
int plugin_checkFDISSET(fd_set*);

#endif

