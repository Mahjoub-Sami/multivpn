#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "globals.h"
#include "debug.h"

#define MAX_LINE 	4096 
#define	MAX_FIELD	4096
#define	MAX_PARAMETER	4096



int ParseConfigFile(char *file_path)
{
   
   FILE *config_file;
   char *temp;
   char line[MAX_LINE + 1];
   //char field[MAX_FIELD+1];
   //char parameter[MAX_PARAMETER + 1];

   int nline;
   
   if ((config_file = fopen(file_path, "r")) == NULL)
   {
      debug_error("Cannot open config file");
      return (-1);
   }
   nline=0;
   while ((fgets(line, MAX_LINE,config_file)) != NULL)
   {
     
      	// Remove new line ...
    	 if ((temp = strchr(line, '\n')) != NULL)
         	*temp = '\0'; 
      	 nline++;

	if (ParseOption(line,nline))
	{
		debug_error("Error Parsing Config File, at line: %d",nline);
		return (-1);
	}
    }
   

   fclose(config_file);
   return 0;
}
int ParseOption(char *option,int nline)
{
	char *command;
	unsigned int n;
	
	debug(2,"Parsing Line: %s",option);
	if (strstr(option,"#"))
	{
		debug(2,"Line %d with comments, skipping parsing",nline);
		return(0);
	}
	if (*option == '\0')
	{
		debug(2,"Empty Line %d, skipping parsing",nline);
		return(0);
	}
	else	debug(3,"Line not empty, Really parsing",nline);
	command=strtok(option," ");
	
	if (!strncmp(command,"remote_host",strlen("remote_host")))
	{
		strncpy(global_v.remote_host,strtok(NULL," "),50);
		debug(2,"remote_host is: %s",global_v.remote_host);
		return 0;
	}
	
	if (!strncmp(command,"remote_port",strlen("remote_port")))
	{
		n=atoi(strtok(NULL," "));
		if (!n)
		 {
		   debug_error("Error parsing remote_port");
		   return (-1);
		 }
		global_v.remote_port=n;
		debug(2,"Remote Port is:",global_v.remote_port);
		return 0;

	}

	if (!strncmp(command,"local_port",strlen("local_port")))
	{
		n=atoi(strtok(NULL," "));
		if (!n)
		{
			debug_error("Error parsing local_port");
			return (-1);
		}
		global_v.local_port=n;
		debug(2,"Local Port is:",global_v.local_port);
		return 0;
	}
 
	if (!strncmp(command,"local_ip",strlen("local_ip")))
	{
		strncpy(global_v.local_ip,strtok(NULL," "),50);
		debug(2,"Local IP is: %s",global_v.local_ip);
		return 0;
	}
	if (!strncmp(command,"local_netmask",strlen("local_nemask")))
	{
		strncpy(global_v.local_netmask,strtok(NULL," "),50);
		debug(2,"Local Netmask is: %s",global_v.local_netmask);
		return 0;
	}

	if (!strncmp(command,"remote_ip",strlen("remote_ip")))
	{
		strncpy(global_v.remote_ip,strtok(NULL," "),50);
		debug(2,"Remote IP is: %s",global_v.remote_ip);
		return 0;
	}

	if (!strncmp(command,"plugin",strlen("plugin")))
	{
		strncpy(global_v.plugin_name,strtok(NULL," "),50);
		debug(2,"Plugin is: %s",global_v.plugin_name);
		return 0;
	}

	if (!strncmp(command,"mode",strlen("mode")))
	{
		command=strtok(NULL," ");
		if (!strncmp(command,"server",strlen("server")))
		{
			global_v.mode=SERVER_MODE;
			debug(1,"Mode Server Activated");
			return 0;
		}
		else
		if (!strncmp(command,"client",strlen("client")))
		{
			global_v.mode=CLIENT_MODE;
			debug(1,"Client Mode Activated");
			return 0;
		}
		else 
		{
				debug(2,"Unkown mode ");
				return (0);
		}
	}

	
	if (!strncmp(command,"sip_port",strlen("sip_port")))
	{
			n=atoi(strtok(NULL," "));
			if (!n)
			{
				debug_error("Error parsing sip_port");
				return (-1);
			}
			global_v.sip_port=n;
			debug(2,"SIP Port is: %d",global_v.sip_port);
			return 0;
	}

	if (!strncmp(command,"sip_transport",strlen("sip_transport")))
	{
		strncpy(global_v.sip_transport,strtok(NULL," "),50);
		debug(2,"SIP Transport is: %s",global_v.sip_transport);
		return 0;
	}
	
	if (!strncmp(command,"sip_proxy",strlen("sip_proxy")))
	{
		strncpy(global_v.sip_proxy,strtok(NULL," "),50);
		debug(2,"SIP Proxy is: %s",global_v.sip_proxy);
		return 0;
	}
	
	if (!strncmp(command,"sip_remoteuri",strlen("sip_remoteuri")))
	{
		strncpy(global_v.sip_remoteuri,strtok(NULL," "),50);
		debug(2,"SIP RemoteUri is: %s",global_v.sip_remoteuri);
		return 0;
	}

	if (!strncmp(command,"sip_fromuri",strlen("sip_fromuri")))
	{
		strncpy(global_v.sip_fromuri,strtok(NULL," "),50);
		debug(2,"SIP FromUri is: %s",global_v.sip_fromuri);
		return 0;
	}

		debug(2,"			Unkown Option parsing config file at line: %d, skipping",nline);
	
	return 0;
}
