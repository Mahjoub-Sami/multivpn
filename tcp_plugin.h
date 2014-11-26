#ifndef _TCP_PLUGIN__H
#define _TCP_PLUGIN__H

#include "globals.h"

int tcp_startServer();		// Parameter: 	tcp port to listen to	
int tcp_startClient();	// Parameters:	remote host, remote port
int tcp_read(int ,char *);
int tcp_send(int ,char *);
int tcp_stop();
int tcp_getSocket();
int tcp_build();
int tcp_buildServer(int);
int tcp_buildClient(char*,int);
int tcp_start();
int tcp_checkParameters();

int tcp_checkLocalPort();
int tcp_checkRemoteHost();
int tcp_checkRemotePort();
int tcp_checkLocalIP();
int tcp_checkRemoteIP();
int tcp_fillFDSET(fd_set*);
int tcp_checkFDISSET(fd_set*);
int tcp_getTunTap();

// Functions Returning Pointers to plugin function handlers
void  *tcp_plugin_getStart();
void  *tcp_plugin_getBuild();
void  *tcp_plugin_getCheckParameters();
void  *tcp_plugin_getTunTap();
void  *tcp_plugin_getFillFDSET();
void  *tcp_plugin_getCheckFDISSET();  
#endif

