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

//定义flags:只写，文件不存在那么就创建，文件长度戳为0
#define FLAGS O_WRONLY | O_CREAT | O_TRUNC
//创建文件的权限，用户读、写、执行、组读、执行、其他用户读、执行
#define MODE S_IRWXU | S_IXGRP | S_IROTH | S_IXOTH

static int sock;

void sig_proccess_client(int signo){
    printf("Catch a exit signal\n");
    close(sock);
    exit (0);
}


void proccess_conn_client(int s){

    int j;
    //Get share memory id
    int shmid = getShm();
    struct shm_mem* mem = (struct shm_mem*)shmat(shmid, NULL, 0);

    //Get Semphore id
    int semid = getSemSet();
    printf("proccess_conn_client begin: semid = %d, shmid = %d\n", semid, shmid);
    fflush(stdout);

    while(1) {
        P(semid,0);
        if(mem->size != 0) {
            j = write(s, &(mem->content[0]), mem->size);
            if(j == 0) printf("error in write data to share memory!\n");
            mem->size = 0;
        }
        V(semid,0);
        usleep(100);
    }
    shmdt(mem);
}

int connect_client(const char* argv[]){
    sock = socket(AF_INET,SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);
    socklen_t len = sizeof(struct sockaddr_in);

    if(connect(sock, (struct sockaddr*)&server, len) < 0 )
    {
        perror("connect");
        return 1;
    }
    return 0;
}

void tcp_sent(const char* argv[])
{
    prctl(PR_SET_NAME,"client_tcp_sent1");
    if(connect_client(argv)!=0){
        printf("Error in connect to client!\n");
        return;
    }
    proccess_conn_client(sock);
    close(sock);
    return;
}

/* Read the content from a file or a libpcap*/
void file_handle(){
    pthread_attr_t attr;
//    struct sched_param sched;
//    int priority = get_thread_priority( &attr );
    prctl(PR_SET_NAME,"client_file_handle");

#if 1  //Read the content from libpcap
    char *filename = "/media/sf_share/exam/pcap";
    int fp = open(filename,FLAGS, MODE);
    if(-1 == fp){
        printf("The file %s can't be open.\n", filename);
        return;
    } else {
        printf("The file %s has been open.\n", filename);
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
        printf("Failed to create SHM!\n");
    }

    //Create Semphore
    int semid = creatSemSet(1);
    printf("shmid is %d\n", semid);
    initSem(semid,0);
    sleep(3);
    char *filename = "/media/sf_share/exam/example";
    int fp = open(filename,O_RDWR,S_IWUSR);
    if(-1 == fp){
        printf("The file %s can't be open.\n", filename);
    } else {
        time_1 = time(NULL);
        printf("The file %s has been open:%ld.\n", filename, time_1);
    }

    while(1) {
        usleep(500);
        P(semid,0);
        if(mem[0] == 0x00){
            bzero(mem, SIZE);
            int num_bytes = read(fp, mem+1, SIZE-8);  // Read the data from the file
            if(num_bytes==0){
                time_2 = time(NULL);
                printf("The file %s has been finished:%ld.\n", filename, time_2);
                close(fp);
                V(semid,0);
                break;
            }else if(num_bytes == -1){
                printf("The file %s has error to be read. errno: %d!\n", filename, errno);
                close(fp);
                V(semid,0);
                break;
            } else {
 //               printf("%d bytes has been read!\n", num_bytes);
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

int main(int argc,const char* argv[])
{

    if(argc != 3)
    {
        printf("Usage:%s [ip] [port]\n",argv[0]);
        return 0;
    }

/* Thread thd_Recv: Receive the data from libpcap
   Thread the_Sent1: Transfer the data to Server via TCP */
    pthread_t thd_Sent1, thd_Recv;

    pthread_attr_t attr_Recv, attr_Sent1;
//    struct sched_param sched_Recv, sched_Sent1;

    int ret_thrd1,ret_thrd2;
    void* retval;

    //Create Share Memory
    int shmid = creatShm();
    char* mem = (char*)shmat(shmid, NULL, 0);
    if(NULL == mem){
        printf("Failed to create SHM!\n");
    } else {
        printf("Main: address of mem: %p\n", mem);
    }

    //Create Semphore
    int semid = creatSemSet(1);
    initSem(semid,0);
    printf("Client_all: semid is %d, shmid is %d\n", semid, shmid);

    int rs;
    rs = pthread_attr_init(&attr_Recv);
    assert( rs == 0 );
    rs =  pthread_attr_init(&attr_Sent1);
    assert( rs == 0 );

    ret_thrd1 = pthread_create(&thd_Sent1, &attr_Sent1, (void *)&tcp_sent, (void*)&argv[0]); // Create thread for TCP_SOCK_1
    ret_thrd2 = pthread_create(&thd_Recv, &attr_Recv, (void *)&file_handle, NULL);  //Create thread for Recv

    if(ret_thrd1 != 0){
        printf("Failed to create TCP_Socket Thread\n");
    } else {
        printf("Success to create TCP_Socket Thread\n");
    }

    if(ret_thrd2 != 0){
        printf("Failed to create File_Handle thread\n");
    } else {
        printf("Success to create File_Handle thread\n");
    }

    int tmp1 = pthread_join(thd_Sent1, &retval);
    printf("TCP_Socket thread return value(tmp) is %d\n", tmp1);
    if (tmp1 != 0) {
        printf("cannot join with TCP_Socket thread\n");
    }
    printf("TCP_Socket thread end\n");

    int tmp2 = pthread_join(thd_Recv, &retval);
    printf("File_Handle thread return value(tmp) is %d\n", tmp2);
    if (tmp2 != 0) {
        printf("cannot join with File_Handle thread\n");
    }
    printf("File_Handle thread end\n");
    return 0;
}