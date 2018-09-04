/*

	twepcrack - Wep Key Cracker

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

	globals.c: global resources.
*/


#include <string.h>
#include "globals.h"


t_global_v global_v;


void Globals_Init()
{
	global_v.verbose=0;
	global_v.debug=10;
	global_v.mode=-1;
	strncpy(global_v.configfile,"none",strlen("none"));
	strncpy(global_v.remote_host,"none",strlen("none"));
	global_v.plugin=-1;
	strncpy(global_v.plugin_name,"none",strlen("none"));
	strncpy(global_v.local_ip,"none",strlen("none"));
	strncpy(global_v.remote_ip,"none",strlen("none"));
	global_v.remote_port=-1;
	global_v.local_port=-1;
}
