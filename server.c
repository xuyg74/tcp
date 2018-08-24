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
#include "comm.h"

//定义flags:只写，文件不存在那么就创建，文件长度戳为0
#define FLAGS O_WRONLY | O_CREAT | O_TRUNC
//创建文件的权限，用户读、写、执行、组读、执行、其他用户读、执行
#define MODE S_IRWXU | S_IXGRP | S_IROTH | S_IXOTH

int startup(int _port,const char* _ip)
{
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0)
    {
        perror("socket");
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
        perror("bind");
        exit(2);
}

    if(listen(sock, 5) < 0)
    {
        perror("listen");
        exit(3);
    }

    return sock;
}

int main(int argc,const char* argv[])
{
    if(argc != 3)
    {
        printf("Usage:%s [loacl_ip] [loacl_port]\n",argv[0]);
        return 1;
    }

    int listen_sock = startup(atoi(argv[2]),argv[1]);

    struct sockaddr_in remote;
    socklen_t len = sizeof(struct sockaddr_in);

    printf("Parent is running.  pid:%d, ppid%d\n",getpid(),getppid());

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
        if(id > 0)
        {//father
            printf(">>>father process<<<");
            close(sock);
        //	while(waitpid(-1, NULL, WNOHANG) > 0);
        }
        else if(id == 0)
        {//child
            printf("get a client, ip:%s, port:%d\n",inet_ntoa(remote.sin_addr),ntohs(remote.sin_port));
            printf("Child is running.  pid:%d, ppid%d\n",getpid(),getppid());
//            close(listen_sock);
            char buf[SIZE];
            char *filename = "/media/sf_share/exam/example_3";
            int fp = open(filename,FLAGS, MODE);
            if(-1 == fp){
                printf("The file %s can't be open.\n", filename);
            } else {
                printf("The file %s has been open.\n", filename);
            }
            while(1)
            {
                bzero(buf, sizeof(buf));
                ssize_t _s = read(sock, buf, sizeof(buf));
                printf("recvd num: %d\n", (int)_s);
                if(_s > 0)
                {
//                    buf[_s] = "\0";
                    int num_bytes = write(fp, buf, _s);
                    if( 0 == num_bytes){
                        printf("Finish the reading!\n");
                        close(fp);
                        break;
                    } else if(-1 == num_bytes){
                        printf("Error in write to the file. errno:%d\n", errno);
                        close(fp);
                        break;
                    } else {
                        printf("aa!\n");
                        
                    }
#if 0
                    buf[_s] = 0;
                    printf("client:%s\n",buf);
                    fflush(stdout);
#endif
                }
                else
                {
                    printf("client is quit!\n");
                    break;
                }
            }
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