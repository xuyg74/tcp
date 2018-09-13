//接收多个数据包 

#include <stdio.h>  
#include <pcap.h>  
#include <arpa/inet.h>  
#include <time.h>  
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <fcntl.h>

#include "pcap_lib.h"
#include "sem_comm.h"
  
#define BUFSIZE 1514

//定义flags:只写，文件不存在那么就创建，文件长度戳为0
#define FLAGS O_WRONLY | O_CREAT | O_TRUNC
//创建文件的权限，用户读、写、执行、组读、执行、其他用户读、执行
#define MODE S_IRWXU | S_IXGRP | S_IROTH | S_IXOTH

//static int cnt = 0;

#if 0  
struct ether_header  
{  
    unsigned char ether_dhost[6];   //dst mac  
    unsigned char ether_shost[6];   //src mac  
    unsigned short ether_type;      //eth type  
};  


int cont_str(const unsigned char *str){
    int i = 0;
    while(*(str+i) != '\0'){
        i++;
    }
    return i;
}
#endif
/*******************************CALLBACK function************************************/  
void get_packet(unsigned char            *argument,
                const struct pcap_pkthdr *packet_header,
                const unsigned char      *packet_content)
{
#if 1
    /* declare pointers to packet headers */
//    struct sniff_ethernet   *ethernet;        /* The ethernet header */
    struct sniff_ip         *ip;              /* The IP header       */
    struct sniff_tcp        *tcp;             /* The TCP header      */ 
//    struct sniff_udp        *udp;             /* The UDP header      */
    unsigned char           *payload;         /* Packet payload      */

    int                     size_ip;
    int                     size_tcp;
    u_short                 ip_length;
//    int                     size_udp;
    int                     size_payload;
    int                     proto_flag=2;// 0=TCP_FLAG; 1=UDP_FLAG
    int                     semid;
    int                     shmid;
//    char*                   mem;
    struct shm_mem          *shm;
#if 0
    char                    *ip_char;
    char                    *ip_tmp;
#endif

//    ethernet = (struct sniff_ethernet*)(packet_content);    

    ip       = (struct sniff_ip*)(packet_content + SIZE_ETHERNET);

    size_ip  = IP_HL(ip)*4;
    ip_length = ntohs(ip->ip_len);
    if (size_ip < 20) {
        printf("   * Invalid IP header length: %u bytes\n", size_ip);
        fflush(stdout);
        return;
    }

    struct user_parm  parm;
    if(argument != NULL){
        memcpy(&parm, argument, sizeof(parm));
        shmid = parm.shmid;
        semid = parm.semid;
        shm = (struct shm_mem*)shmat(shmid, NULL, 0);
        printf("size_ip:%d, shmid:%d, semid: %d\n", ip_length, shmid, semid);
        fflush(stdout);
    } else {
        printf("arguemnt is null!\n");
        return;
    }
#if 0
    ip_char = (char *)malloc(ip_length*sizeof(char));
    ip_tmp = (char *)ip;
    memset(ip_char, 0, ip_length*sizeof(char));
    for(int i = 0; i < ip_length; i++){
        *(ip_char+i) = *(ip_tmp+i) + '0';
    }
#endif
    switch(ip->ip_p) {
        case IPPROTO_TCP://useful
            printf("   Protocol: TCP\n");
            proto_flag=PROTOCOL_TCP;
            break;
 
        case IPPROTO_UDP://useful
            printf("   Protocol: UDP\n");
            proto_flag=PROTOCOL_UDP;
            return;
 
        case IPPROTO_ICMP://useless
            printf("   Protocol: ICMP\n");
            proto_flag=PROTOCOL_ICMP;
            break;
 
        case IPPROTO_IP: //useless
            printf("   Protocol: IP\n");
            proto_flag=PROTOCOL_IP;
            return;
 
        default:
            printf("   Protocol: unknown\n");
            proto_flag=PROTOCOL_OTHER;
            return;
    }

#if 1
    char *filename = "/media/sf_share/exam/pcap";
    int fp = open(filename,FLAGS, MODE);
    if(-1 == fp){
        printf("The file %s can't be open.\n", filename);
    } else {
        printf("The file %s has been open.\n", filename);
    }

    int num_bytes = 0;
    num_bytes = write(fp, packet_content, ip_length+SIZE_ETHERNET);
    if(num_bytes == 0){
        printf("failed to write to files!\n");
    } else {
        close(fp);
    }
#endif


    if (proto_flag == PROTOCOL_TCP) {
        tcp = (struct sniff_tcp *) (packet_content + SIZE_ETHERNET + size_ip);
        size_tcp = TH_OFF (tcp) * 4;
        if (size_tcp < 20) {
            printf ("   * Invalid TCP header length: %u bytes\n", size_tcp);
            return;
        }

        printf("   From: %s\n", inet_ntoa(ip->ip_src));
        printf("   To: %s\n", inet_ntoa(ip->ip_dst));
        printf("   Src port  : %d\n", ntohs (tcp->th_sport));
        printf("   Dst port  : %d\n", ntohs (tcp->th_dport));
        printf("   Seq number: %d\n", ntohl (tcp->th_seq));
        int fin=0;
        if(tcp->th_flags & TH_FIN) fin=1;
        printf("   FIN       : %d\n", fin); 
 
        /* define/compute tcp payload (segment) offset */
        payload = (unsigned char *) (packet_content + SIZE_ETHERNET + size_ip + size_tcp);
 
        /* compute tcp payload (segment) size */
        size_payload = ntohs (ip->ip_len) - (size_ip + size_tcp);
        printf("   TCP size_payload: %d\n", size_payload);
        P(semid,0);
        memcpy(&(shm->content[0]), payload, size_payload);
        shm->size = size_payload;
        V(semid,0);
    } else if (proto_flag == PROTOCOL_ICMP) {
        if(shm->size == 0) {
            P(semid,0);
#if 0            
            char *ip_tmp = (char *)ip;
            printf("   ICMP size: %d, ip size:%d\n", ip_length+SIZE_ETHERNET, cont_str(ip));
            printf("   ip:%02x02%x02%x02%x02%x02%x02%x02%x%02x02%x02%x02%x02%x02%x02%x02%x%02x02%x02%x02%x02%x02%x02%x02%x%02x02%x02%x02%x02%x02%x02%x02%x\n", 
                ip_tmp[0],ip_tmp[1],ip_tmp[2],ip_tmp[3],ip_tmp[4],ip_tmp[5],ip_tmp[6],ip_tmp[7],
                ip_tmp[8],ip_tmp[9],ip_tmp[10],ip_tmp[11],ip_tmp[12],ip_tmp[13],ip_tmp[14],ip_tmp[15],
                ip_tmp[16],ip_tmp[17],ip_tmp[18],ip_tmp[19],ip_tmp[20],ip_tmp[21],ip_tmp[22],ip_tmp[23],
                ip_tmp[24],ip_tmp[25],ip_tmp[26],ip_tmp[27],ip_tmp[28],ip_tmp[29],ip_tmp[30],ip_tmp[31]);
#endif
            memcpy(&(shm->content[0]), packet_content, ip_length+SIZE_ETHERNET);
            shm->size = ip_length+SIZE_ETHERNET;
            V(semid,0);
            printf("if: shm->size is %d\n", shm->size);
        } else {
            printf("else: shm->size is %d\n", shm->size);
        }
    }
//    free(ip_char);
#endif

#if 0
    unsigned char *mac_string;              //  
    struct ether_header *ethernet_protocol;  
    unsigned short ethernet_type;           //以太网类型  
    cnt++;
    printf("packet %d: --------------------------------------------\n", cnt);
    printf("%s\n", ctime((time_t *)&(packet_header->ts.tv_sec))); //转换时间
    ethernet_protocol = (struct ether_header *)packet_content;
      
    mac_string = (unsigned char *)(ethernet_protocol->ether_shost);//获取源mac地址
    printf("Mac Source Address is %02x:%02x:%02x:%02x:%02x:%02x\n",
        *(mac_string+0),
        *(mac_string+1),
        *(mac_string+2),
        *(mac_string+3),
        *(mac_string+4),
        *(mac_string+5));
    mac_string = (unsigned char *)(ethernet_protocol->ether_dhost);//获取目的mac
    printf("Mac Destination Address is %02x:%02x:%02x:%02x:%02x:%02x\n",
        *(mac_string+0),
        *(mac_string+1),
        *(mac_string+2),
        *(mac_string+3),
        *(mac_string+4),
        *(mac_string+5));

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
#endif
}  
  
//int main(int argc, char *argv[])
int pcap_lib(int shmid, int semid)
{  
    char error_content[200];    //出错信息
    pcap_if_t *alldevs;
    pcap_if_t *d;
    int i = 0;
    pcap_t *pcap_handle;
    struct user_parm  parm;
    parm.shmid = shmid;
    parm.semid = semid;
      
    //Get the network interface
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

    /* construct a filter */
    struct bpf_program filter_pgm;
    char *filter =  "ether dst 08:00:27:77:6e:51";
    pcap_compile(pcap_handle, &filter_pgm, filter, 1, 0);
    pcap_setfilter(pcap_handle, &filter_pgm);

#if 1
    if(pcap_loop(pcap_handle,-1,get_packet,(u_char *)&parm) < 0)
    {  
        perror("pcap_loop");  
    }  
#endif
    pcap_freealldevs(alldevs);
    pcap_close(pcap_handle);
    pcap_handle = NULL;

    return 0;  
}