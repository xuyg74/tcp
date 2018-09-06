//接收多个数据包 

#include <stdio.h>  
#include <pcap.h>  
#include <arpa/inet.h>  
#include <time.h>  
#include <stdlib.h>
#include <unistd.h>
  
#define BUFSIZE 1514

static int cnt = 0;
  
struct ether_header  
{  
    unsigned char ether_dhost[6];   //目的mac  
    unsigned char ether_shost[6];   //源mac  
    unsigned short ether_type;      //以太网类型  
};  
  
/*******************************回调函数************************************/  
void ethernet_protocol_callback(unsigned char *argument,const struct pcap_pkthdr *packet_heaher,const unsigned char *packet_content)  
{  
    unsigned char *mac_string;              //  
    struct ether_header *ethernet_protocol;  
    unsigned short ethernet_type;           //以太网类型  
    cnt++;
    printf("packet %d: --------------------------------------------\n", cnt);  
    printf("%s\n", ctime((time_t *)&(packet_heaher->ts.tv_sec))); //转换时间  
    ethernet_protocol = (struct ether_header *)packet_content;  
      
    mac_string = (unsigned char *)ethernet_protocol->ether_shost;//获取源mac地址  
    printf("Mac Source Address is %02x:%02x:%02x:%02x:%02x:%02x\n",*(mac_string+0),*(mac_string+1),*(mac_string+2),*(mac_string+3),*(mac_string+4),*(mac_string+5));  
    mac_string = (unsigned char *)ethernet_protocol->ether_dhost;//获取目的mac  
    printf("Mac Destination Address is %02x:%02x:%02x:%02x:%02x:%02x\n",*(mac_string+0),*(mac_string+1),*(mac_string+2),*(mac_string+3),*(mac_string+4),*(mac_string+5));  
      
    ethernet_type = ntohs(ethernet_protocol->ether_type);//获得以太网的类型  
    printf("Ethernet type is :%04x\n",ethernet_type);  
    switch(ethernet_type)  
    {  
        case 0x0800:printf("The network layer is IP protocol\n\n\n");break;//ip  
        case 0x0806:printf("The network layer is ARP protocol\n\n\n");break;//arp  
        case 0x0835:printf("The network layer is RARP protocol\n\n\n");break;//rarp  
        default:break;  
    }  
    usleep(1000);  
}  
  
int pcap(int argc, char *argv[])  
{  
    char error_content[200];    //出错信息
    pcap_if_t *alldevs;
    pcap_if_t *d;
    int i = 0;
    pcap_t *pcap_handle;  
//    unsigned char *mac_string;                
//    unsigned short ethernet_type;           //以太网类型  
//    char *net_interface = NULL;                 //接口名字  
//    struct pcap_pkthdr protocol_header;  
//    struct ether_header *ethernet_protocol;
      
    //获取网络接口
    if(pcap_findalldevs(&alldevs, error_content) == -1){
        printf("Error in pcap_findalldevs \n");
        return -1;
    }

    for(d = alldevs; d ; d=d->next){
        i++;
        printf("name:%s\n", d->name);
        if(d->description){
            printf("description: %s\n", d->description);
        }
    }
    if(i == 0){
        printf("No interface found! Make sure libpcap is installed!\n");
    }

#if 1
    if((pcap_handle = pcap_open_live(alldevs->name,BUFSIZE,1,0,error_content)) == NULL)  {
        printf("Can't open the first device %s,\nerror:%s!\n",alldevs->name, error_content);
        pcap_freealldevs(alldevs);
        return -1;
    } else {
        printf("device %s has been open!\n", alldevs->name);
    }
#endif

#if 1          
    if(pcap_loop(pcap_handle,-1,ethernet_protocol_callback,NULL) < 0)  
    {  
        perror("pcap_loop");  
    }  
#endif
    pcap_freealldevs(alldevs);
    pcap_close(pcap_handle);
    pcap_handle = NULL;

    return 0;  
}