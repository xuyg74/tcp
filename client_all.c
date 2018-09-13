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
#include "comm.h"
#include <fcntl.h>
#include <errno.h>
#include<pthread.h>
#include<time.h>

#include<sys/wait.h>
#include"sem_comm.h"
#include"pcap_lib.h"

static int sock;

void sig_proccess_client(int signo){
    printf("Catch a exit signal\n");
    close(sock);
    exit (0);
}


#if 1
void proccess_conn_client(int s){
    //Share Memory
    int shmid = getShm();
    struct shm_mem* mem = (struct shm_mem*)shmat(shmid, NULL, 0);

    //Semphore
    int semid = getSemSet();
    printf("proccess_conn_client begin: semid = %d, shmid = %d\n", semid, shmid);
    fflush(stdout);
//    char buf[1024];
//    int j = 999;

    while(1)
    {
        P(semid,0);
        if(mem->size != 0){
//            write(s, mem+1, strlen(mem)-1);
            int j = write(s, &(mem->content[0]), mem->size);
            printf("length of mem = %d,j = %d\n", mem->size,j);
//            fflush(stdout);
            mem->size = 0;
        }
//        fflush(stdout);
//        usleep(100);
        V(semid,0);
    }
    shmdt(mem);
}
#endif

void socket_handle(const char* argv[])
{

    sock = socket(AF_INET,SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("socket");
        return;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);
    socklen_t len = sizeof(struct sockaddr_in);

    if(connect(sock, (struct sockaddr*)&server, len) < 0 )
    {
        perror("connect");
        return;
    }
    proccess_conn_client(sock);
    close(sock);
    return;
}

/* Read the content from a file or a libpcap*/
void file_handle(){
    int shmid = getShm();
    int semid = getSemSet();
    pcap_lib(shmid, semid);


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
    pthread_t thread_1, thread_2;

    int ret_thrd1,ret_thrd2;
    void* retval;

#if 1   //Read the content from the libpcap
    int shmid = creatShm();
    char* mem = (char*)shmat(shmid, NULL, 0);
    if(NULL == mem){
        printf("Failed to create SHM!\n");
    } else {
        printf("file_handle: length of mem: %ld\n", sizeof(mem));
    }

    //Create Semphore
    int semid = creatSemSet(1);
    printf("file_handle: semid is %d, shmid is %d\n", semid, shmid);
    initSem(semid,0);
#endif

    ret_thrd1 = pthread_create(&thread_1, NULL, (void *)&socket_handle, (void*)&argv[0]);
    ret_thrd2 = pthread_create(&thread_2, NULL, (void *)&file_handle, NULL);

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

    int tmp1 = pthread_join(thread_1, &retval);
    printf("TCP_Socket thread return value(tmp) is %d\n", tmp1);
    if (tmp1 != 0) {
        printf("cannot join with TCP_Socket thread\n");
    }
    printf("TCP_Socket thread end\n");

    int tmp2 = pthread_join(thread_2, &retval);
    printf("File_Handle thread return value(tmp) is %d\n", tmp2);
    if (tmp2 != 0) {
        printf("cannot join with File_Handle thread\n");
    }
    printf("File_Handle thread end\n");

    return 0;
}
