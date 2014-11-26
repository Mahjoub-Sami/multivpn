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

	main.c: application entry point.

*/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/wait.h>
#include <sys/time.h>



#include "globals.h"
#include "parser.h"
#include "plugin.h"

void MainLoop();

void ShowVersion(void){
	printf("\n%s\n\n",PROGRAM_VERSION);
	exit(0);
}

void ShowBanner(void){
        printf("%s Multiprotocol VPN tool (%s).\n%s\n\n",PROGRAM_NAME, PROGRAM_VERSION, PROGRAM_AUTHOR);
}

void UsageMsg(char *programName){
	ShowBanner();
	printf("Usage: %s [OPTIONS]... \n", programName);
	printf("  Options:\n");
        printf("    --debug <debuglevel>	prints debug information\n");
        printf("    -c, --configfile <file>     use specified config file, Required\n");
	printf("    -h, --help		        display this help and exit\n");
	printf("    -V, --version	        output version information and exit\n");
	printf("\n");
	exit(1);
}

void ShowHelp(void){
	char programName[2048];
	sprintf(programName,"%s %s",PROGRAM_NAME, PROGRAM_VERSION);
	UsageMsg(programName);
}


int main(int argc, char **argv){
	int optionSelected=1; 
	int opt;
	
	struct option long_options[]={
		{"version",0,NULL,'V'},
		{"help",0,NULL,'h'},
		{"debug",1,NULL,'d'},
		{"verbose",0,NULL,'v'},
		{"configfile",1,NULL,'c'},
		{NULL,0,NULL,0}
	};
	int option_index=0;

	if (argc<1)
		UsageMsg(argv[0]);
	
	Globals_Init();


	while ((opt = getopt_long(argc, argv,"c:Vvhd:", long_options, &option_index)) != EOF) {
		switch (opt) {
			case 'V':
				ShowVersion();
		    		break;
			case 'v':
				global_v.verbose=1;	
				break;
			case 'h':
				ShowHelp();
				break;
			case 'd':
		 	   if (!optarg || optarg[0]=='-') UsageMsg(argv[0]);
				global_v.debug=atoi(optarg);
				if (!global_v.debug) UsageMsg(argv[0]);	
				break;
			case 'c':
		 	   if (!optarg || optarg[0]=='-') UsageMsg(argv[0]);
				strncpy(global_v.configfile,optarg,200);	
				break;

			default:
				printf("ERROR: unknown option.\n\n");
				UsageMsg(argv[0]);
		}
	}
	if (!strncmp(global_v.configfile,"none",strlen("none")))
		UsageMsg(argv[0]);
	ShowBanner();

	debug(1,"Debug mode initialized");
	
	debug(1,"Parsing Config File...");

	if 	( ParseConfigFile(global_v.configfile) )
			debug_error("Error Parsing Config File");
	else		debug(1,"Succes Parsing Config File\n");

	debug(1,"MAIN THREAD: Verifying Parameters");
	if	( plugin_checkConfig(global_v) )
	{
		debug_error("Error Checking Plugin Parameters");
		exit(-1);
	}
	else		debug(1,"Success Checking Plugin Parameters");

	debug(1,"MAIN THREAD: Building Plugin");
	if  ( plugin_build() )
	{
		debug_error("Failed Building Plugin, Exiting");
		exit(-1);
	}
	else	debug(1,"Success Full Plugin Build");
	
	debug(1,"Main Thread: Starting Plugin");
	plugin_start();
	debug(1,"Main Thread: Entering Main Loop");
	MainLoop();

	debug(1,"Main Thread: Finishing the program...");


	return 0;
}


void MainLoop()
{
	int finish;
	int nbytes;
	int fdTun;
	int maxFd;
	char buffer[BLOCK_SIZE];
	fd_set	setReading;
	int pluginReadSocket;
	
	fdTun=tun_getFile();
	if (fdTun>global_v.pipe_from_plugin[0])
		maxFd=fdTun+1;
	else	maxFd=global_v.pipe_from_plugin[0]+1;
	
	debug(1,"MAIN Thread: Enteryng loop");
	
	finish=0;
	while (!finish)
	{
		// Preparing FD set 
		plugin_fillFDSET(&setReading);
		FD_SET(fdTun,&setReading);
		
		// Now Blocking
		debug(3,"MAIN THREAD: Select Blocked");
		select(maxFd,&setReading,NULL,NULL,NULL);
		
		// Now Processing correct file descriptor 
		debug(3,"MAIN THREAD: Select unblocked");

		if (FD_ISSET(fdTun,&setReading))
		{
			debug(3,"Main Thread: Message is from tundriver");
			nbytes=read(fdTun,buffer,BLOCK_SIZE-1);
			if (nbytes<=0)
				debug(3,"MAIN Thread: Failed Reading from tundriver");
			else
			{
				debug(3,"MAIN Thread: Read %d bytes from tundriver",nbytes);
				nbytes=write(global_v.pipe_to_plugin[1],buffer,nbytes);
				if (nbytes<=0)
					debug(3,"MAIN Thread: Failed Writing to pipe");
				else	debug(3,"MAIN Thread: Wrotten %d bytes to plugin",nbytes);

			}
		}
		else 
		if ( pluginReadSocket=plugin_checkFDISSET(&setReading) )
		{
			debug(3,"MAIN Thread: Message is from plugin socket");
			nbytes=read(pluginReadSocket,buffer,BLOCK_SIZE);
			if (nbytes<=0)
				debug(3,"MAIN Thread: Failed Reading from pluginSocket");
			else
			{
				debug(3,"MAIN Thread: Read %d bytes from pluginSocket",nbytes);
				nbytes=write(fdTun,buffer,nbytes);
				if (nbytes<=0)
					debug(3,"MAIN THread: Failed writting to tundriver");
				else	debug(3,"MAIN Thread: Wrotten %d bytes to tundriver",nbytes);
				   
				
			}
			
		}
		else debug(3,"Extrange select...");
				
		FD_ZERO(&setReading);
				
	}
	//
	debug(1,"MAIN Thread: End loop");
}
