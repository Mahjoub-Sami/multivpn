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

	tuntap.c 	: 	Access tuntap file (iface)
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
#include <linux/if_tun.h>

#include "tuntap.h"
#include "debug.h"

#define TUNDEV "/dev/net/tun"

static char iface_name[256];
static int tfd;

int tun_open()
{
	return(tun_alloc(TUNDEV));
}
int tun_alloc (const char *path)
{
  struct ifreq ifr;
  if ((tfd = open(path, O_RDWR )) < 0)
  {
	  perror("tuntap failed open: ");
	  return -1;
  }

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TAP|IFF_NO_PI;

  if (ioctl(tfd, TUNSETIFF, (void *) &ifr) < 0)
  {
       close(tfd);
       tfd = -1;
       return errno;
  }
  fprintf(stderr,"Open: %s\n",ifr.ifr_name);
  strncpy(iface_name,ifr.ifr_name,100);
  return 0;
}

int tun_write(int numbytes,char*data)
{
	int result;
	result=write(tfd, data, numbytes);
	return result;
}
int tun_read(int numbytes,char*data)
{
	int result;
	result=read(tfd,data,numbytes);	
	return result;
}
int tun_getFile()
{
	return tfd;
}

int tun_UP()
{
     struct ifreq ifr;
     int ctrl_sock;

     debug(4,"Trying to bring tunnel interface UP");
     
     ctrl_sock = socket(PF_INET, SOCK_DGRAM, 0);

      if(ctrl_sock < 0 )
       {
        debug_error("Cannot Allocate Socket");
        return -1;
       }

      memset(&ifr, 0, sizeof(ifr));
      strcpy(ifr.ifr_ifrn.ifrn_name, iface_name);

      
    /* Get the current flags */
    if( ioctl(ctrl_sock, SIOCGIFFLAGS, &ifr) == -1 ) 
    {
          
 	debug_error("Cannot get current Interfaces flags");
	return -1;
    }

    /* Turn on the UP flag */
    ifr.ifr_flags |= IFF_UP;
    if( ioctl(ctrl_sock, SIOCSIFFLAGS, &ifr) == -1 ) 
    {
	debug_error("Cannot bring interface up");
	return -1;
    }

    return 0;    
}
int tun_setIP(char *ip_str)
{
     struct in_addr ipaddr;
     struct ifreq ir;
     struct sockaddr_in *sin = (void *) &ir.ifr_ifru.ifru_addr;
     int ctrl_sock;	  
	     
     
     debug(3,"Trying to set ip: %s",ip_str);
     
     if (!inet_aton(ip_str,&ipaddr) )
     {
	
	     debug_error("Invalid IP Address");
	     return -1;
     }	  
     debug(4,"TUNTAP SET IP addr = %08x\n\n",ipaddr.s_addr);

     ctrl_sock = socket(PF_INET, SOCK_DGRAM, 0);
     
     if(ctrl_sock < 0 )
     {
      	     debug_error("Cannot Allocate Socket");
	     return -1;
      }
     

      memset(&ir, 0, sizeof(ir));
      strcpy(ir.ifr_ifrn.ifrn_name, iface_name);

      sin->sin_family = AF_INET;
      sin->sin_addr.s_addr = ipaddr.s_addr;
      sin->sin_port = 0;
			
      if ( ioctl(ctrl_sock,SIOCSIFADDR,&ir)   < 0   ) 
      {
              debug_error("Failed to Assign IP Address");
              return -1;
      }

      close(ctrl_sock);
      
      return 0;
}

int tun_setNETMASK(char *netmask_str)
{
     struct in_addr ipaddr;
     struct ifreq ir;
     struct sockaddr_in *sin = (void *) &ir.ifr_ifru.ifru_addr;
     int ctrl_sock;	  
	     
     
     debug(3,"Trying to set netmask: %s",netmask_str);
     
     if (!inet_aton(netmask_str,&ipaddr) )
     {
	
	     debug_error("Invalid NETMASK Address");
	     return -1;
     }	  
     debug(4,"TUNTAP SET NETMASK addr = %08x\n\n",ipaddr.s_addr);

     ctrl_sock = socket(PF_INET, SOCK_DGRAM, 0);
     
     if(ctrl_sock < 0 )
     {
      	     debug_error("Cannot Allocate Socket");
	     return -1;
      }
     

      memset(&ir, 0, sizeof(ir));
      strcpy(ir.ifr_ifrn.ifrn_name, iface_name);

      sin->sin_family = AF_INET;
      sin->sin_addr.s_addr = ipaddr.s_addr;
      sin->sin_port = 0;
			
      if ( ioctl(ctrl_sock,SIOCSIFNETMASK,&ir)   < 0   ) 
      {
              debug_error("Failed to Assign NETMASK Address");
              return -1;
      }

      close(ctrl_sock);
      
      return 0;
}
