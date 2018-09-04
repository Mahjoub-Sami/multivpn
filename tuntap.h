#ifndef TUNTAP__H
#define TUNTAP__H

int tun_setNETMASK(char *netmask_str);
int tun_alloc (const char *path);
int tun_open();
char *tun_getIface();
int tun_getFile();
int tun_write(int,char*);
int tun_read(int,char*);
int tun_setIP(char *);
int tun_UP();
int tun_DOWN();

#endif

