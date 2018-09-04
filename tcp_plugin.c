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

	tcp_plugin:	PLUGIN via TCP connection 
*/
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sysexits.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <string.h>
#include "generictunel.h"
#include "tcp_plugin.h"
#include "tuntap.h"
#include "debug.h"

static int socketConnected;
static int socketRemote;
static int socketServer;
static int modeServer;
static int remoteClientConnected;
static struct sockaddr_in serverAddress;
static socklen_t sizeSockAddr;

//
static int clientID;	// ID for using generictunnel functions, client mode has only one connection

int tcp_buildClient(char *remoteHost,int remotePort)
{
  struct sockaddr_in destinoDir;
  struct hostent *he;
  modeServer=0;

  if ((socketRemote=socket(AF_INET, SOCK_STREAM,0))==-1)
   {
        perror("socket");
        return -1;
   }

  if ((he=gethostbyname(remoteHost))==NULL)
   {
     debug_error("Cannot Resolve Host");
     return(-1);
   }


  destinoDir.sin_family=AF_INET;
  destinoDir.sin_port=htons(remotePort);
  destinoDir.sin_addr.s_addr=INADDR_ANY;
  destinoDir.sin_addr = *((struct in_addr *)he->h_addr);
  bzero(&(destinoDir.sin_zero),8);
 
  debug(1,"Connecting to %s ...",remoteHost);

  if (connect(socketRemote, (struct sockaddr *)&destinoDir,sizeof (struct sockaddr)) == -1)
  {
   debug_error("Can Connect to RemoteHost!");
   return(-1);
  }
  else debug(1,"Connected to RemoteHost");

  clientID=StoreConnectionInformation(TCP_PLUGIN,NULL);
  if (!clientID)
  {
	debug_error("Cannot store connection information");
	return -1;
  }
    return(0);
}
int tcp_buildServer(int localPort)
{
//  int tamanioServer;
//  int tamanioDestino;

  modeServer=1;
  remoteClientConnected=0;
  
  if ((socketServer=socket(AF_INET, SOCK_STREAM,0))==-1)
      {
        debug_error("socket creation failed");
        return -1;
      }

 serverAddress.sin_family=AF_INET;
 serverAddress.sin_port=htons(localPort);
 serverAddress.sin_addr.s_addr=INADDR_ANY;
 bzero(&(serverAddress.sin_zero),8);
 sizeSockAddr = sizeof (struct sockaddr);

 if
 ( bind(socketServer,(struct sockaddr *)&serverAddress, sizeSockAddr) ==-1)
 {
  debug_error("bind port error");
  return(-1);
 }

 if ( listen(socketServer,1)==-1)
 {
  debug_error("listen error");
  return(-1);
 }


  debug(1,"TCP Plugin: Listen for connection...");
  return 0;
}

int tcp_read(int numbytes,char *data)
{
     int recibido;
     if (modeServer)
	  recibido=recv(socketConnected,data,numbytes,0);
     else recibido=recv(socketRemote,data,numbytes,0);
     

     return recibido;
}
int tcp_send(int numbytes,char *data)
{
    int enviado;

    if (modeServer)
	  enviado=send(socketConnected,data,numbytes,0);
    else  enviado=send(socketRemote,data,numbytes,0);

    return enviado;
}

int tcp_stop()
{
	if (modeServer)
	{
		if (remoteClientConnected)
			close(socketConnected);
		close(socketServer);
		remoteClientConnected=0;
	}
	else
	{
		close(socketRemote);
	}
	return 0;
}

int tcp_getSocket()
{
	if (modeServer)
		return (socketConnected);
	else	return (socketRemote);
}
int tcp_build()
{
	debug(1,"TCP Plugin: Building");
	if (tun_open())
	{
		debug_error("Tun_open failed");
		return -1;
	}
	else debug(3,"TCP_PLUGIN Succesfull open tun");
	debug(2,"Building generic options for tcp_plugin");
	
	
	switch(global_v.mode)
	{
		case SERVER_MODE:
			debug(2,"TCP Plugin building as server");
			return (tcp_buildServer(global_v.local_port));
			break;
		case CLIENT_MODE:
			debug(2,"TCP Plugin Building as client");
			if ( tcp_buildClient(global_v.remote_host ,global_v.remote_port) )
			{
				debug(2,"TCP Plugin Failed Building as client");		
				return -1;
			}
			break;
		default:
			return -1;
			
	
	}
	return 0;
}

int tcp_start()
{
	debug(1,"TCP Plugin Starting");
	debug(2,"Setting UP IP Addressing");
	tun_setIP(global_v.local_ip);
	tun_setNETMASK("255.255.255.0");
	tun_UP();
	switch(global_v.mode)
	{
		case SERVER_MODE:
				tcp_startServer();
				break;
		case CLIENT_MODE:
				tcp_startClient();
				break;
	}
	return 0;
}

int tcp_startClient()
{
    char buffer[BLOCK_SIZE];
    int nBytes;
    fd_set  setReading;
    int maxfd;
    
    debug(2,"TCP Plugin: Starting Client Thread");
    if (socketRemote>global_v.pipe_to_plugin[0])
        maxfd = socketRemote+1;
    else    maxfd = global_v.pipe_to_plugin[0]+1;
    while (1)
    {
                FD_SET(global_v.pipe_to_plugin[0],&setReading);
                FD_SET(socketRemote,&setReading);
                debug(3,"TCP Plugin: Blocking now, reading from socket or from pipe");
                select(maxfd,&setReading,NULL,NULL,NULL);
        if (FD_ISSET(socketRemote,&setReading))
        {  
            // Message is from socket
                // Read From socket
            nBytes=recv(socketRemote,buffer,BLOCK_SIZE-1,0);
            if (nBytes<=0)
                debug(3,"TCP Plugin: Error Reading From Socket");
            else    
            {
                debug(3,"TCP Plugin: Read %d bytes from socket",nBytes);
                // ..... then Write to Pipe
                nBytes=write(global_v.pipe_from_plugin[1],buffer,nBytes);
                if (nBytes<=0)
                    debug(3,"TCP Plugin: Failed Writing to Pipe");
                else    debug(3,"TCP Plugin: Write %d bytes to pipe",nBytes);
            }
        }
        else
        if (FD_ISSET(global_v.pipe_to_plugin[0],&setReading))
        {
            // Message is from pipe
                // Read From Pipe
            nBytes=read(global_v.pipe_to_plugin[0],buffer, BLOCK_SIZE-1);
            if (nBytes<=0)
                debug(3,"TCP Plugin: failed reading From Pipe");
            else    
            {
                debug(3,"TCP Plugin: Read %d bytes from pipe",nBytes);
                // ..... then send to socket
                nBytes=send(socketRemote,buffer,nBytes,0);
                if (nBytes<=0)
                    debug(3,"TCP Plugin: Failed Sending to Socket");
                else    debug(3,"TCP Plugin: Write %d bytes to Socket",nBytes);
            }
        }
        
    }
}


int tcp_startServer()
{
	char buffer[BLOCK_SIZE];
	int nBytes;
	fd_set	setReading;
	int maxfd;

	debug(2,"TCP Plugin: Starting Server Thread");

	socketConnected=accept(socketServer,(struct sockaddr *)&serverAddress,&sizeSockAddr);
	remoteClientConnected=1;
        debug(3,"TCP Plugin: remote host connected!...");
	
	if (socketConnected>global_v.pipe_to_plugin[0])
		maxfd=socketConnected+1;
	else	maxfd=global_v.pipe_to_plugin[0]+1;
	
	while (1)
	{
		while (1)
		{
			FD_SET(global_v.pipe_to_plugin[0],&setReading);
			FD_SET(socketConnected,&setReading);
			debug(3,"TCP Plugin: Blocking now, reading from socket or from pipe");
			select(maxfd,&setReading,NULL,NULL,NULL);
		    debug(3,"TCP Plugin: UNBLOCKED!");
	
			if ( FD_ISSET(socketConnected,&setReading) )
			{
				//	MESSAGE IS FROM REMOTE SOCKET
					// Read from socket
				nBytes=recv(socketConnected,buffer,BLOCK_SIZE-1,0);
				if (nBytes <=0)
					debug(3,"TCP PLugin: Failed Reading From Socket");
				else	
				{
					debug(3,"TCP Plugin: Read %d bytes from socket",nBytes);
					// ... then Write to Pipe
					nBytes=write(global_v.pipe_from_plugin[1],buffer,nBytes);
					if (nBytes<=0)
						debug(3,"TCP Plugin: Failed Writing to Pipe");
					else	debug(3,"TCP PLugin: Write %d bytes to pipe",nBytes);
				}
			}
			else
			if  ( FD_ISSET(global_v.pipe_to_plugin[0],&setReading) )
			{
				//	MESSAGE IS FROM PIPE
				nBytes=read(global_v.pipe_to_plugin[0],buffer,BLOCK_SIZE-1);
					// Read from pipe
				if (nBytes<=0)
					debug(3,"TCP Plugin: Failed Read from pipe");
				else
				{
					debug(3,"TCP Plugin: Read %d bytes from pipe",nBytes);
					// .... then write to socket
					nBytes=send(socketConnected,buffer,nBytes,0);
					if (nBytes<=0)
						debug(3,"TCP Plugin: Failed sending to Socket");
					else	debug(3,"TCP Plugin: Wrotten %d bytes to socket",nBytes);
				}
			}
		}
	    
	}
}
int tcp_fillFDSET(fd_set *set)
{
	debug(3,"TCP PLUGIN: Filling fdset");
	switch (global_v.mode)
	{
		case SERVER_MODE:
				debug(4,"TCP PLUGIN: Filling as server");
				break;
		case CLIENT_MODE:
				debug(4,"TCP PLUGIN: Filling as client");
				FD_SET(GetSocketRecvQueue(TCP_PLUGIN,clientID),set);
				break;
			
	}
	return 0;
}
int tcp_checkParameters()
{
	debug(1,"TCP Plugin Checking parameters...");
	if (tcp_checkMode())
	{
		debug_error("TCP Plugin Requires server mode or client mode");
		return -1;
	}
	switch (global_v.mode)
	{
		case SERVER_MODE:
			if ( tcp_checkLocalPort() || tcp_checkLocalIP() || tcp_checkRemoteIP())
			{
				debug_error("TCP Plugin Server gMode requires: local_port, local_ip, remote_ip");
				return -1;
			}
			else 
			{	
				debug(2,"TCP Plugin Server mode parameters OK");
				return 0;
			}
			break;
		case CLIENT_MODE:
			if ( tcp_checkRemoteHost() || tcp_checkRemotePort() || tcp_checkLocalIP() || tcp_checkRemoteIP())
			{
				debug_error("TCP Plugin Client Mode requires: remote, remote_port, local_ip, remote_ip");
			}
			break;
		default:
			return -1;
	}

	return 0;
}

int tcp_checkMode()
{
	return (global_v.mode==-1);
}
int tcp_checkLocalPort()
{
	return (global_v.local_port==-1);
}
int tcp_checkRemoteHost()
{
	return (!strncmp(global_v.remote_host,"none",strlen("none")));	
}
int tcp_checkRemotePort()
{
	return (global_v.remote_port==-1);	
}
int tcp_checkLocalIP()
{
	return (!strncmp(global_v.local_ip,"none",strlen("none")));	
}
int tcp_checkRemoteIP()
{
	return (!strncmp(global_v.remote_ip,"none",strlen("none")));	
}
int tcp_checkFDISSET(fd_set *set)
{
	switch ( global_v.mode )
	{
		case CLIENT_MODE:
			return(GetSocketRecvQueue(TCP_PLUGIN,clientID) );
			break;
		case SERVER_MODE:
			break;
		default:
			return 0;
	}
	return 0;
}
int tcp_getTunTap()
{
	return (tun_getFile());

}
void *tcp_plugin_getStart()
{
	return tcp_start;
}
void  *tcp_plugin_getCheckParameters()
{
	return tcp_checkParameters;
}
void *tcp_plugin_getFillFDSET()
{
	return tcp_fillFDSET;
}
void *tcp_plugin_getCheckFDISSET()
{
	return tcp_checkFDISSET;
}
void  *tcp_plugin_getTunTap()
{
	return tcp_getTunTap;
}
void *tcp_plugin_getBuild()
{
	return tcp_build;
}
