#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <errno.h>
#include<pthread.h>
#include<time.h>
#include <sys/prctl.h>

#include<sys/wait.h>
#include"sem_comm.h"
#include"pcap_lib.h"
#include "comm.h"
#include "pthread_tools.h"

#include <pthread.h>
#include <sched.h>
#include <assert.h>
#include "dbg.h"
#include "readconf.h"

//定义flags:只写，文件不存在那么就创建，文件长度戳为0
#define FLAGS O_WRONLY | O_CREAT | O_TRUNC
//创建文件的权限，用户读、写、执行、组读、执行、其他用户读、执行
#define MODE S_IRWXU | S_IXGRP | S_IROTH | S_IXOTH

static int sock[MAX_TCP_SENT];

void sig_proccess_client(int signo){
//    DEBUG_INFO("Catch a exit signal\n");
    close(sock[0]);
    exit (0);
}


void proccess_conn_client(int s){

    int j;
    //Get share memory id
    int shmid = getShm();
    struct shm_mem* mem = (struct shm_mem*)shmat(shmid, NULL, 0);

    //Get Semphore id
    int semid = getSemSet();
    DEBUG_INFO("proccess_conn_client begin: semid = %d, shmid = %d\n", semid, shmid);
    fflush(stdout);

    while(1) {
        P(semid,0);
        if(mem->size != 0) {
            j = write(s, &(mem->content[0]), mem->size);
            if(j == 0) ERR_INFO("error in write data to share memory!\n");
            mem->size = 0;
        }
        V(semid,0);
        usleep(100);
    }
    shmdt(mem);
}

int connect_client(TCP_PARM* argv){
    sock[argv->serial] = socket(AF_INET,SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(&(argv->port[0])));
    server.sin_addr.s_addr = inet_addr(&(argv->ip_addr[0]));
    socklen_t len = sizeof(struct sockaddr_in);

    if(connect(sock[argv->serial], (struct sockaddr*)&server, len) < 0 )
    {
        perror("connect");
        return 1;
    }
    return 0;
}

void tcp_sent(TCP_PARM* argv)
{
    prctl(PR_SET_NAME,"client_tcp_socket1");
    if(connect_client(argv)!=0){
        ERR_INFO("Error in connect to client!\n");
        return;
    } else {
        DEBUG_INFO("TCP Socket connected!\n");
    }
    proccess_conn_client(sock[argv->serial]);
    close(sock[argv->serial]);
    return;
}

/* Read the content from a file or a libpcap*/
void file_handle(){
//    pthread_attr_t attr;
//    struct sched_param sched;
//    int priority = get_thread_priority( &attr );
    prctl(PR_SET_NAME,"client_file_handle");

#if 1  //Read the content from libpcap
    char *filename = "/media/sf_share/exam/pcap";
    int fp = open(filename,FLAGS, MODE);
    if(-1 == fp){
        ERR_INFO("The file %s can't be open.\n", filename);
        return;
    } else {
        DEBUG_INFO("The file %s has been open.\n", filename);
    }
    int shmid = getShm();
    int semid = getSemSet();
    pcap_lib(shmid, semid, fp);
    close(fp);
#endif

#if 0  //Read the content from a file
    time_t time_1, time_2;
    int shmid = creatShm();
    char* mem = (char*)shmat(shmid, NULL, 0);
    if(NULL == mem){
        DEBUG_INFO("Failed to create SHM!\n");
    }

    //Create Semphore
    int semid = creatSemSet(1);
    DEBUG_INFO("shmid is %d\n", semid);
    initSem(semid,0);
    sleep(3);
    char *filename = "/media/sf_share/exam/example";
    int fp = open(filename,O_RDWR,S_IWUSR);
    if(-1 == fp){
        DEBUG_INFO("The file %s can't be open.\n", filename);
    } else {
        time_1 = time(NULL);
        DEBUG_INFO("The file %s has been open:%ld.\n", filename, time_1);
    }

    while(1) {
        usleep(500);
        P(semid,0);
        if(mem[0] == 0x00){
            bzero(mem, SIZE);
            int num_bytes = read(fp, mem+1, SIZE-8);  // Read the data from the file
            if(num_bytes==0){
                time_2 = time(NULL);
                DEBUG_INFO("The file %s has been finished:%ld.\n", filename, time_2);
                close(fp);
                V(semid,0);
                break;
            }else if(num_bytes == -1){
                DEBUG_INFO("The file %s has error to be read. errno: %d!\n", filename, errno);
                close(fp);
                V(semid,0);
                break;
            } else {
 //               DEBUG_INFO("%d bytes has been read!\n", num_bytes);
 //               fflush(stdout);
                mem[0] = 0x1;
                V(semid,0);
            }
        }
    }

    shmdt(mem);
    destoryShm(shmid);
#endif
    return;
}

int main(int argc, const char* argv[])
{

    if(argc != 2)
    {
        ERR_INFO("Usage:%s network_file\n",argv[0]);
        return 0;
    }

/* Thread thd_Recv: Receive the data from libpcap
   Thread the_Sent1: Transfer the data to Server via TCP */
    pthread_t thd_Sent[MAX_TCP_SENT], thd_Recv;
    pthread_attr_t attr_Recv, attr_Sent[MAX_TCP_SENT];
    int ret_thd_sent[MAX_TCP_SENT],ret_thrd2;
    void* retval;
    int rs;
    int tmp[MAX_TCP_SENT];
    TCP_PARM  tcp_parm[MAX_TCP_SENT];

    //Create Share Memory
    int shmid = creatShm();
    char* mem = (char*)shmat(shmid, NULL, 0);
    if(NULL == mem){
        ERR_INFO("Failed to create SHM!\n");
    } else {
        DEBUG_INFO("Main: address of mem: %p\n", mem);
    }

    //Create Semphore
    int semid = creatSemSet(1);
    initSem(semid,0);
    DEBUG_INFO("Client_all: semid is %d, shmid is %d\n", semid, shmid);

    TCP_CONF    conf;
    memset(&conf, 0, sizeof(conf));
    readconfig(argv[1], &conf);

    int socket_num = conf.tcp_num;

    rs = pthread_attr_init(&attr_Recv);
    assert( rs == 0 );

    for(int i = 0; i < socket_num; i++){
        rs = pthread_attr_init(&attr_Sent[i]);
        memcpy(&(tcp_parm[i].ip_addr[0]),&(conf.ip_addr[i][0]), 64*sizeof(char));
        memcpy(&(tcp_parm[i].port[0]),&(conf.port[i][0]), 8*sizeof(char));
        tcp_parm[i].serial = i;
        assert(rs == 0);
        ret_thd_sent[i] = pthread_create(&thd_Sent[i], &attr_Sent[i], (void *)&tcp_sent, (void*)&tcp_parm[i]);
        if(ret_thd_sent[i] != 0){
            ERR_INFO("Failed to create TCP_Socket Thread\n");
        } else {
            DEBUG_INFO("Success to create TCP_Socket Thread, i= %d\n", i);
        }
    }

    ret_thrd2 = pthread_create(&thd_Recv, &attr_Recv, (void *)&file_handle, NULL);  //Create thread for Recv
    if(ret_thrd2 != 0){
        ERR_INFO("Failed to create File_Handle thread\n");
    } else {
        DEBUG_INFO("Success to create File_Handle thread\n");
    }

    for(int i = 0; i < socket_num; i++){
        tmp[i] = pthread_join(thd_Sent[i], &retval);
        if (tmp[i] != 0) {
            ERR_INFO("cannot join with TCP_Socket thread, return value:%d\n", tmp[i]);
        } else {
            DEBUG_INFO("TCP_Socket thread return value(tmp) is %d\n", tmp[i]);
        }
        DEBUG_INFO("TCP_Socket thread end, i = %d\n", i);
    }

    int tmp2 = pthread_join(thd_Recv, &retval);
    DEBUG_INFO("File_Handle thread return value(tmp) is %d\n", tmp2);
    if (tmp2 != 0) {
        ERR_INFO("cannot join with File_Handle thread\n");
    }
    DEBUG_INFO("File_Handle thread end\n");
    return 0;
}