#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>

#include<signal.h>

extern void sig_pipe(int signo){
    printf("Catch a sig_pipe signal, signo = %d\n", signo);
    _exit(0);
}
