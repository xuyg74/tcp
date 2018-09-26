#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "readconf.h"
#include "comm.h"
#include "dbg.h"

//定义flags:只写，文件不存在那么就创建，文件长度戳为0
#define FLAGS O_WRONLY | O_CREAT | O_TRUNC
//创建文件的权限，用户读、写、执行、组读、执行、其他用户读、执行
#define MODE S_IRWXU | S_IXGRP | S_IROTH | S_IXOTH

int startup(int _port,const char* _ip)
{
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0)
    {
        ERR_INFO("socket\n");
        exit(1);
    }

    int opt=1;
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons( _port);
    local.sin_addr.s_addr = inet_addr(_ip);
    socklen_t len = sizeof(local);

    if(bind(sock,(struct sockaddr*)&local , len) < 0)
    {
        ERR_INFO("bind\n");
        exit(2);
}

    if(listen(sock, 5) < 0)
    {
        ERR_INFO("listen\n");
        exit(3);
    }

    printf("Start succeed!\n");

    return sock;
}

/* Functions to handle the data in the server node*/
void server_handle(int sock){
    char buf[SIZE];
    char *filename = "/media/sf_share/exam/server";
    int fp = open(filename,FLAGS, MODE);
//    int i = 0;
    if(-1 == fp){
        ERR_INFO("The file %s can't be open.\n", filename);
        return;
    } else {
        DEBUG_INFO("The file %s has been open.\n", filename);
    }
    while(1) {
        bzero(buf, sizeof(buf));
        ssize_t _s = read(sock, buf, sizeof(buf));
//        DEBUG_INFO("recvd num: %d\n", (int)_s);
        if(_s > 0) {
            int num_bytes = write(fp, buf, _s);
            if( 0 == num_bytes){
                DEBUG_INFO("Finish the reading!\n");
                close(fp);
                break;
            } else if(-1 == num_bytes){
                ERR_INFO("Error in write to the file. errno:%d\n", errno);
                close(fp);
                break;
            } else {
//                i++;
//                DEBUG_INFO("write %d to files!\n", i);                        
            }
        } else {
            ERR_INFO("client is quit!\n");
            break;
        }
    }
    return;
}

int main(int argc,const char* argv[])
{
    TCP_CONF    conf;
    memset(&conf, 0, sizeof(conf));
    int socket_num;
    int listen_sock[MAX_TCP_SENT];
    struct sockaddr_in remote[MAX_TCP_SENT];
    socklen_t len = sizeof(struct sockaddr_in);
    int sock[MAX_TCP_SENT];

    if(argc != 2)
    {
        ERR_INFO("Usage:%s /etc/network/server\n",argv[0]);
        return 1;
    }

    OUT_INFO("Parent is running.  pid:%d, ppid:%d\n",getpid(),getppid());

    readconfig(argv[1], &conf);
    socket_num = conf.tcp_num;

    for(int i = 0; i < socket_num; i++){
        listen_sock[i] = startup(atoi(&(conf.port[i][0])),&(conf.ip_addr[i][0]));
        while(1){
            sock[i] = accept(listen_sock[i], (struct sockaddr*)&remote[i], &len);
            if(sock[i]<0){
                ERR_INFO("error in accept, i = %d\n", i);
                continue;
            }
            pid_t id = fork();
            if(id > 0){
                DEBUG_INFO(">>>Farther Process<<<\n");
                close(sock[i]);
            } else if(id == 0){
                OUT_INFO("get a client, ip:%s, port:%d\n",inet_ntoa(remote[i].sin_addr),ntohs(remote[i].sin_port));
                OUT_INFO("Child is running.  pid:%d, ppid%d\n",getpid(),getppid());
                server_handle(sock[i]);
                close(listen_sock[i]);
                close(sock[i]);
            } else {
                ERR_INFO("fork error, i = %d\n", i);
                return 2;
            }
        }
    }
    return 0;
}
    
#if 0
//    struct sockaddr_in remote;
//    socklen_t len = sizeof(struct sockaddr_in);
    while(1)
    {
        int sock = accept(listen_sock, (struct sockaddr*)&remote, &len);
        if(sock < 0)
        {
            perror("accept");
            continue;
        }

        //fork a child process for handling traffic while a connetion is established
        pid_t id = fork();
        if(id > 0) {//father
            DEBUG_INFO(">>>father process<<<");
            close(sock);
        //	while(waitpid(-1, NULL, WNOHANG) > 0);
        } 
        else if(id == 0) {
        //child
            OUT_INFO("get a client, ip:%s, port:%d\n",inet_ntoa(remote.sin_addr),ntohs(remote.sin_port));
            OUT_INFO("Child is running.  pid:%d, ppid%d\n",getpid(),getppid());
            server_handle(sock);
            close(listen_sock);
            close(sock);
        }
        else
        {
            perror("fork");
            return 2;
        }
    }
    return 0;
}
#endif