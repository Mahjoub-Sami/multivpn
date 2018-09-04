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

	generictunnel.h: generic protocol tunnel API.
*/


#ifndef	_GENERICTUNNEL_H
#define	_GENERICTUNNEL_H

#define NUMBER_PLUGINS	1
#define MAXINT	32767

#include <sys/types.h>
#include <sys/socket.h>


int StoreConnectionInformation(int pnumber, void *data);
int GetSocketRecvQueue(int pnumber,int id);

#endif
