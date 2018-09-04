/*

	multivpn - Multiprotocol VPN tool 

	Copyright (C) 2004  Gorka Gorrotxategi - zgor; Jose Ignacio Sanchez Martin - Topo[LB]

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software Foundation,
	Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  

	---------

	globals.h: global resources.
*/


#ifndef	_GLOBALS_H
#define	_GLOBALS_H



#define	PROGRAM_NAME	"MultiVPN - Multiprotocol VPN tool"
#define PROGRAM_VERSION	"v0.0.1"
#define PROGRAM_AUTHOR	"Gorka Gorrotxategi - zgor <zgor@int80h.net>, Jose Ignacio Sanchez Martin - Topo[LB] <topolb@users.sourceforge.net>"

#define MAX_IN_CONNECTIONS	50
#define NUMBER_PLUGINS		1

#define	TCP_PLUGIN		0
#define SIP_PLUGIN		1

#define	SERVER_MODE		0
#define	CLIENT_MODE		1

#define	BLOCK_SIZE		1505

typedef struct{
	int verbose;
	int debug;
	int mode;
	char configfile[256];
	char remote_host[256];
	int  plugin;			// Active Plugin number
	char plugin_name[256];		// Active Plugin Name
	char local_ip[256];
	char local_netmask[256];
	char remote_ip[256];
	unsigned int  remote_port;
	unsigned int  local_port;
	// SIP Plugin specifics:
	unsigned int sip_port;
	char sip_transport[256];
	char sip_remoteuri[256];
	char sip_fromuri[256];
	char sip_proxy[256];

	int pipe_from_plugin[2];		// Pipe For Reading from plugin
	int pipe_to_plugin[2];			// Pipe For Writing to plugin
}t_global_v;

extern t_global_v global_v;


void Globals_Init();

#endif
