#ifndef _READCONF_
#define _READCONF_

#define MAX_TCP_SENT    2

typedef struct port_conf
{
	int  tcp_num;
	char ip_addr[MAX_TCP_SENT][64];
	char port[MAX_TCP_SENT][8];
} TCP_CONF;

typedef struct port_parm
{
	int serial;
	char ip_addr[64];
	char port[8];
} TCP_PARM;

int readconfig(const char *config_file, TCP_CONF *tcp);

#endif