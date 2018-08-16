#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>

#include<signal.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include "signal_handle.h"
#include "comm.h"

extern void sig_proccess(int signo);
extern void sig_pipe(int signo);

static int sock;

extern void sig_proccess(int signo){
    printf("Catch a sig_proccess signal, signo = %d\n", signo);
    close(sock);
    exit (0);
}

void sig_proccess_client(int signo){
    printf("Catch a exit signal\n");
    close(sock);
    exit (0);
}
#if 0
void proccess_conn_client(int s){
    ssize_t _s = 0;
    char buffer[1024];
    while(1){
        printf("send###");
        fflush(stdout);
        _s = read(0, buffer, sizeof(buffer)-1);
        buffer[_s] = 0;
        write(s,buffer,_s);
    }
}
#endif

#if 1
void proccess_conn_client(int s){
    int shmid = getShm();
    char* mem = (char*)shmat(shmid, NULL, 0);
//    char buf[1024];

    while(1)
    {
//        ssize_t _s = read(0, mem, sizeof(mem));
//        printf("shm length:%d\n", (int)strlen(mem));
        int j = 999;
//        memset(buf,'\0', sizeof(buf));
//        memcpy(buf, mem, strlen(mem));
        j = write(s, mem, strlen(mem));
//        printf("j = %d, %s\n",j, mem);
        printf("j = %d\n", j);
//        fflush(stdout);
        usleep(100);
    }
    shmdt(mem);
}
#endif

int main(int argc,const char* argv[])
{

    if(argc != 3)
    {
        printf("Usage:%s [ip] [port]\n",argv[0]);
        return 0;
    }

    signal(SIGINT, sig_proccess);
    signal(SIGPIPE, sig_pipe);

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
        return 2;
    }
#if 0
    char buf[1024];
    while(1)
    {
        printf("send>>>");
        fflush(stdout);

        ssize_t _s = read(0, buf, sizmakeeof(buf)-1);
        buf[_s] = 0;
        write(sock, buf, _s);
    }
#endif
    proccess_conn_client(sock);
    close(sock);
    return 0;
}