/*

	multivpn - Multiprotocol VPN tool 

	Copyright (C) 2004 Gorka Gorrotxategi - zgor; Jose Ignacio Sanchez Martin - Topo[LB]

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

	plugin.c:	Plugin handler
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "tcp_plugin.h"
#include "sip_plugin.h"
#include "globals.h"
#include "debug.h"


// Active Plugin Functions Pointers
static void* (*active_plugin_start)(void *);
static int (*active_plugin_build)(void);
//static int (*active_plugin_check_parameters)(t_global_v);
static int (*active_plugin_check_parameters)(void);
static int (*active_plugin_getTunTap)(void);
static int (*active_plugin_fillFDSET)(fd_set*);
static int (*active_plugin_checkFDISSET)(fd_set*);
// Active Plugin Main Thread
static pthread_t active_plugin_thread;


int plugin_checkConfig(t_global_v variables)
{

	debug(1, "User selected plugin is: %s",global_v.plugin_name);
	if (!strncmp(global_v.plugin_name,"tcp",strlen("tcp")))
	{
		debug(1,"PLUGIN: tcp plugin selected");
		global_v.plugin=TCP_PLUGIN;
		active_plugin_start=tcp_plugin_getStart();
		active_plugin_build=tcp_plugin_getBuild();
		active_plugin_check_parameters=tcp_plugin_getCheckParameters();
		active_plugin_fillFDSET=tcp_plugin_getFillFDSET();
		active_plugin_checkFDISSET=tcp_plugin_getCheckFDISSET();
		active_plugin_getTunTap=tcp_plugin_getTunTap();
		return (active_plugin_check_parameters());
	}
	
	if (!strncmp(global_v.plugin_name,"sip",strlen("sip")))
	{
		debug(1,"PLUGIN: SIP plugin selected, lets fight with RFC3261");
		global_v.plugin=SIP_PLUGIN;
		debug(1,"PLUGIN: SIP - getting pointers to functions");
	   active_plugin_start=sip_plugin_getStart();
		active_plugin_build=sip_plugin_getBuild();
		active_plugin_check_parameters=sip_plugin_getCheckParameters();
		//active_plugin_fillFDSET=sip_plugin_getFillFDSET();
		//active_plugin_checkFDISSET=sip_plugin_getCheckFDISSET();
		//active_plugin_getTunTap=sip_plugin_getTunTap();
		
		debug(1,"Calling PLUGIN SIP: Returning active plugin check parameters");
	
		return (active_plugin_check_parameters());

	}


	// In case plugin does not exists
		debug_error("Plugin: %s not in this source code",global_v.plugin_name);
		return(-1);
	
}
int plugin_build()
{
	if (pipe(global_v.pipe_from_plugin) || pipe(global_v.pipe_to_plugin) )
	{
		perror("Plugin: Failed Building Pipes ");
		return -1;
	}
	
	debug(1,"PLUGIN: Building Active - calling active_plugin_build");
	if ( active_plugin_build() )
		return -1;
	
	
	return 0;
	
}
int plugin_start()
{
	if (active_plugin_start)
	{
		// Starting Active Plugin Main Thread
		debug(2,"creating plugin thread now");
		pthread_create( &active_plugin_thread, NULL, active_plugin_start, NULL);
		return 0;
	}
	else 
	{
		debug_error("Active Plugin Start Call not set!");
		return (-1);
	}

}

int plugin_getTunTap()
{
	return 0;
}

void plugin_stop()
{
	debug(1,"stopping plugin");
}

int plugin_fillFDSET(fd_set *set)
{
	debug(3,"Plugin filling fdset for blocking");
	active_plugin_fillFDSET(set);
	return 0;
}

int plugin_checkFDISSET(fd_set *set)
{
//	int s;
	
	debug(3,"Plugin Checking read sockets");
	
	return 0;
}
