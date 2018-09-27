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
#include "dbg.h"
  
#define BUFSIZE 1514

static int rec_pkt = 0;
static int cnt_copy = 0;
static int cnt_o = 0;
static int output = 1;
static int cnt    = 0;

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
//    int                     shmid;
//    int                     fp;
//    char*                   mem;
    struct shm_mem          *shm;
#if 0
    char                    *ip_char;
    char                    *ip_tmp;
#endif

//    ethernet = (struct sniff_ethernet*)(packet_content);    

    ip = (struct sniff_ip*)(packet_content + SIZE_ETHERNET);

    size_ip  = IP_HL(ip)*4;
    ip_length = ntohs(ip->ip_len);

#if 0    
    if (size_ip < 20) {
        ERR_INFO("get_packet:   * Invalid IP header length: %u bytes\n", size_ip);
        fflush(stdout);
        return;
    }
#endif

    struct user_parm  parm;
    if(argument != NULL){
        memcpy(&parm, argument, sizeof(parm));
//        fp    = parm.fp;
//        shmid = parm.shmid;
        semid = parm.semid;
        shm = (struct shm_mem*)parm.shmid;
//        DEBUG_INFO("size_ip:%d, shmid:%d, semid: %d\n", ip_length, shmid, semid);
//        fflush(stdout);
    } else {
        ERR_INFO("arguemnt is null!\n");
        return;
    }

    switch(ip->ip_p) {
        case IPPROTO_TCP://useful
            DEBUG_INFO("   Protocol: TCP\n");
            proto_flag=PROTOCOL_TCP;
            break;
 
        case IPPROTO_UDP://useless
            DEBUG_INFO("   Protocol: UDP\n");
            proto_flag=PROTOCOL_UDP;
            return;
 
        case IPPROTO_ICMP://useful
            DEBUG_INFO("   Protocol: ICMP\n");
            rec_pkt++;
            proto_flag=PROTOCOL_ICMP;
            DEBUG_INFO("rec_pkt: %d\n", rec_pkt);
            fflush(stdout);
            break;
 
        case IPPROTO_IP: //useless
            DEBUG_INFO("   Protocol: IP\n");
            proto_flag=PROTOCOL_IP;
            return;
 
        default:
            DEBUG_INFO("   Protocol: unknown\n");
            proto_flag=PROTOCOL_OTHER;
            return;
    }

#if 0 //Save the received data into one file for verification
    int num_bytes = 0;
    num_bytes = write(fp, packet_content, ip_length+SIZE_ETHERNET);
    if(num_bytes == 0){
        DEBUG_INFO("failed to write to files!\n");
    }
#endif

#if 1
    if (proto_flag == PROTOCOL_TCP) {
        tcp = (struct sniff_tcp *) (packet_content + SIZE_ETHERNET + size_ip);
        size_tcp = TH_OFF (tcp) * 4;
        if (size_tcp < 20) {
            DEBUG_INFO ("   * Invalid TCP header length: %u bytes\n", size_tcp);
            return;
        }

        DEBUG_INFO("   From: %s\n", inet_ntoa(ip->ip_src));
        DEBUG_INFO("   To: %s\n", inet_ntoa(ip->ip_dst));
        DEBUG_INFO("   Src port  : %d\n", ntohs (tcp->th_sport));
        DEBUG_INFO("   Dst port  : %d\n", ntohs (tcp->th_dport));
        DEBUG_INFO("   Seq number: %d\n", ntohl (tcp->th_seq));
        int fin=0;
        if(tcp->th_flags & TH_FIN) fin=1;
        DEBUG_INFO("   FIN       : %d\n", fin); 
 
        /* define/compute tcp payload (segment) offset */
        payload = (unsigned char *) (packet_content + SIZE_ETHERNET + size_ip + size_tcp);
 
        /* compute tcp payload (segment) size */
        size_payload = ntohs (ip->ip_len) - (size_ip + size_tcp);
        DEBUG_INFO("   TCP size_payload: %d\n", size_payload);
        P(semid,0);
        memcpy(&(shm->content[0]), payload, size_payload);
        shm->size = size_payload;
        V(semid,0);
    } else if (proto_flag == PROTOCOL_ICMP) {
        cnt_o++;
        DEBUG_INFO("cnt_o:%d!\n", cnt_o);
        if((shm->size + ip_length)< CONTENT_LENGTH) {
            P(semid,0);
            memcpy(&(shm->content[shm->size]), packet_content, ip_length+SIZE_ETHERNET);
            shm->size += ip_length+SIZE_ETHERNET;
            V(semid,0);
            cnt_copy++;
            if((cnt_o != cnt_copy)&&(output == 1)){
                ERR_INFO("rec_pkt: %d, cnt_o: %d, cnt_copy:%d!\n", rec_pkt, cnt_o, cnt_copy);
                fflush(stdout);
                output = 0;
                usleep(200);
            }
            if((CONTENT_LENGTH - shm->size)<0x10000){
                cnt++;
                ERR_INFO("%d usleep, rec_pkt: %d, cnt_o: %d, cnt_copy:%d!\n", cnt, rec_pkt, cnt_o, cnt_copy);
                fflush(stdout);
                usleep(200);
            }
//            DEBUG_INFO("if: shm->size is %d\n", shm->size);
        } else {
            ERR_INFO("else: shm->size is %d, rec_pkt: %d, cnt_o: %d, cnt_copy:%d!\n", shm->size, rec_pkt, cnt_o, cnt_copy);
            fflush(stdout);
            usleep(100);
        }
    }
//    free(ip_char);
#endif

#if 0
    unsigned char *mac_string;              //  
    struct ether_header *ethernet_protocol;  
    unsigned short ethernet_type;           //以太网类型  
    cnt++;
    DEBUG_INFO("packet %d: --------------------------------------------\n", cnt);
    DEBUG_INFO("%s\n", ctime((time_t *)&(packet_header->ts.tv_sec))); //转换时间
    ethernet_protocol = (struct ether_header *)packet_content;
      
    mac_string = (unsigned char *)(ethernet_protocol->ether_shost);//获取源mac地址
    DEBUG_INFO("Mac Source Address is %02x:%02x:%02x:%02x:%02x:%02x\n",
        *(mac_string+0),
        *(mac_string+1),
        *(mac_string+2),
        *(mac_string+3),
        *(mac_string+4),
        *(mac_string+5));
    mac_string = (unsigned char *)(ethernet_protocol->ether_dhost);//获取目的mac
    DEBUG_INFO("Mac Destination Address is %02x:%02x:%02x:%02x:%02x:%02x\n",
        *(mac_string+0),
        *(mac_string+1),
        *(mac_string+2),
        *(mac_string+3),
        *(mac_string+4),
        *(mac_string+5));

    ethernet_type = ntohs(ethernet_protocol->ether_type);//获得以太网的类型
    DEBUG_INFO("Ethernet type is :%04x\n",ethernet_type);
    switch(ethernet_type)
    {  
        case 0x0800:DEBUG_INFO("The network layer is IP protocol\n\n\n");break;//ip
        case 0x0806:DEBUG_INFO("The network layer is ARP protocol\n\n\n");break;//arp
        case 0x0835:DEBUG_INFO("The network layer is RARP protocol\n\n\n");break;//rarp
        default:break;
    }
    usleep(1000);
#endif
}  
  
//int main(int argc, char *argv[])
int pcap_lib(int shmid, int semid, int fp)
{  
    char error_content[200];    //出错信息
    pcap_if_t *alldevs;
    pcap_if_t *d;
    int i = 0;
    pcap_t *pcap_handle;
    struct user_parm  parm;
    parm.shmid = shmat(shmid, NULL, 0);
    parm.semid = semid;
    parm.fp    = fp;

    //Get the network interface
    if(pcap_findalldevs(&alldevs, error_content) == -1){
        ERR_INFO("Error in pcap_findalldevs \n");
        return -1;
    }

    for(d = alldevs; d ; d=d->next){
        i++;
        DEBUG_INFO("name:%s\n", d->name);
        if(d->description){
            DEBUG_INFO("description: %s\n", d->description);
        }
    }
    if(i == 0){
        DEBUG_INFO("No interface found! Make sure libpcap is installed!\n");
    }

    if((pcap_handle = pcap_open_live("enp0s9",BUFSIZE,1,0,error_content)) == NULL)  {
        ERR_INFO("Can't open the first device %s,\nerror:%s!\n",alldevs->name, error_content);
        pcap_freealldevs(alldevs);
        return -1;
    } else {
        DEBUG_INFO("device %s has been open!\n", "enp0s9");
    }

    /* construct a filter */
    struct bpf_program filter_pgm;
    char *filter =  "dst host 192.168.2.4";
    pcap_compile(pcap_handle, &filter_pgm, filter, 1, 0);
    pcap_setfilter(pcap_handle, &filter_pgm);

    // Read and handle the packets from libpcap
    if(pcap_loop(pcap_handle,-1,get_packet,(u_char *)&parm) < 0)
    {  
        perror("pcap_loop");  
    }  

    pcap_freealldevs(alldevs);
    pcap_close(pcap_handle);
    pcap_handle = NULL;

    return 0;  
}