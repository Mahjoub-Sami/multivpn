#ifndef _SIP_PLUGIN__H
#define _SIP_PLUGIN__H

#include "globals.h"

/*
 * FUNCIONES IMPLEMENTEADAS POR TCP PLUGIN
 * (como guia de las que tendriamos que montar en SIP)
 *
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
*/
int sip_build();
int sip_start();
void* sip_loop_sip_events(void *);
void sip_loop_tun_events();
int sip_envia_datos();


// Functions Returning Pointers to plugin function handlers
void  *sip_plugin_getStart();
void  *sip_plugin_getBuild();
void  *sip_plugin_getCheckParameters();
void  *sip_plugin_getTunTap();
void  *sip_plugin_getFillFDSET();
void  *sip_plugin_getCheckFDISSET();  
#endif

