#include <stdio.h>
#include <string.h>
#include "dbg.h"
#include "readconf.h"

int parseString(char *origin, char *ipaddr, char *port){
	if( NULL == origin){
		ERR_INFO("origin is MULL!\n");
		return 1;
	}
	char* pChar   = origin;
	int num_nul   = 0;
	int to_ip     = 0;
    int ip_size   = 0;
    int port_size = 0;
    while(*pChar != '\0'){
    	if(*pChar != ' '){
    		(to_ip == 0)?ip_size++:port_size++;
    	} else {
    		num_nul++;
    		to_ip = 1;
    	}
    	pChar++;
    }
    memcpy(ipaddr, origin, ip_size);
    *(ipaddr+ip_size) = '\0';
    memcpy(port, origin+num_nul+ip_size, port_size);
    *(port+port_size) = '\0';
    DEBUG_INFO("ipaddr: %s, port: %s, num_nul: %d\n", ipaddr,port, num_nul);
    return 0;
}

int readconfig(const char *config_file, TCP_CONF *tcp){
    char szTest[64] = {0};

    if(NULL == config_file)
    {
        ERR_INFO("config_file is NULL\n");
        return 1;
    }

    FILE *fp = fopen(config_file,"r");
    if(NULL == fp){
        ERR_INFO("Failed to open the file %s\n", config_file);
        return 1;
    }
    while(1){
        fgets(szTest, 63,fp);
        if(!feof(fp)){
            DEBUG_INFO("string from the file: %s",szTest);
            parseString(szTest, &tcp->ip_addr[tcp->tcp_num][0], &tcp->port[tcp->tcp_num][0]);
            tcp->tcp_num++;
        } else {
            break;
        }
    }
    fclose(fp);
    DEBUG_INFO("The End!\n");
    return 0;
}