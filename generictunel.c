/*

	multivpn - Wep Key Cracker

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

	generictunnel.c: generic protocol tunnel API.
*/


#include "generictunel.h"
#include "globals.h"

// This is the struct of data stored per connection in each plugin.
typedef struct{
	void *connectiondata;		// The plugin can cast this void pointer and store custom data
	int sendsocket;	// The packet send queue
	int recvsocket;	// The packet receive queue		
	unsigned short active;	// Whether this connection its been used right now, or not
}tnconnection;

// In the "struct" of each plugin, there is an static array that keeps another "struct" of data per connection.
typedef struct{
	tnconnection connections[MAX_IN_CONNECTIONS];		
}tnplugindata;


// Each plugin has its own struct with data
tnplugindata plugindata[NUMBER_PLUGINS];



// Store data for an specific connection.
// Returns a connection ID
//   > Some tunnels do not have a socket as the connection identification (DNS tunneling). The idea is to always use a virtual ID to identify a connection. Then if a socket is used in this protocol, the plugin can store it as connection data and retrieve it each time an operation with a virtual connection ID is requested.
int StoreConnectionInformation(int pnumber, void *data){
	int i=0;
	for (i=1; i++; i<MAXINT && plugindata[pnumber].connections[i].active);
	// We did that to reuse empty holes
	if (i==MAXINT) return 0;
	plugindata[pnumber].connections[i].active=1;
	plugindata[pnumber].connections[i].connectiondata=data;
	plugindata[pnumber].connections[i].sendsocket=socket(AF_INET, SOCK_STREAM,0);
	plugindata[pnumber].connections[i].recvsocket=socket(AF_INET, SOCK_STREAM,0);
	return i;
}

void RemoveStoredConnectionInformation(int pnumber, int id){
	plugindata[pnumber].connections[id].active=1;
	if (plugindata[pnumber].connections[id].connectiondata) free(plugindata[pnumber].connections[id].connectiondata);
}

// Retrieve data for an specific ID
void *RetrieveConnectionInformation(int pnumber, int id){
	return plugindata[pnumber].connections[id].connectiondata;
}


// Is this function possible?
//unsigned int GetNumberOfPacketsInSendQueue(int pnumber, int id){
//	return plugindata[pnumber].connections[id].sendqueue->totalPackets;
//}

// Is this function possible?
//unsigned int GetNumberOfPacketsInRecvQueue(int pnumber,int id)
//{
//	return plugindata[pnumber].connections[id].readqueue->totalPackets;
//}


// Gets the first packet in read queue
//  > Returns 0 if error. Packet size otherwise
int GetPacketInRecvQueue(int pnumber, int id, unsigned char *data, unsigned int datasize){
	return recv(plugindata[pnumber].connections[id].recvsocket,data,datasize,0);			
}


// Gets the first packet in send queue
//  > Returns 0 if error. Packet size otherwise
int GetFirstPacketInSendQueue(int pnumber, int id, unsigned char *data, unsigned int datasize){
	return recv(plugindata[pnumber].connections[id].sendsocket,data,datasize,0);		
}

// Gets the first packet in read queue
//  > Returns 0 if error. Packet size otherwise
int PutPacketInRecvQueue(int pnumber, int id, unsigned char *data, unsigned int datasize){
	return send(plugindata[pnumber].connections[id].recvsocket,data,datasize,0);			
}

// Gets the first packet in read queue
//  > Returns 0 if error. Packet size otherwise
int PutPacketInSendQueue(int pnumber, int id, unsigned char *data, unsigned int datasize){
	return send(plugindata[pnumber].connections[id].sendsocket,data,datasize,0);			
}

// Gets the socket send queue
int GetSocketRecvQueue(int pnumber,int id)
{
	return (plugindata[pnumber].connections[id].recvsocket);
}
// Gets the socket recv queue
int GetSocketSendQueue(int pnumber,int id)
{
	return (plugindata[pnumber].connections[id].sendsocket);
}
