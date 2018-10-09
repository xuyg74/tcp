#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>

#include<signal.h>
#include "dbg.h"
#include "sem_comm.h"
#include "comm.h"

extern long rcv_cnt;
extern long sent_cnt;
extern long server_cnt;

extern void sig_pipe(int signo){
    OUT_INFO("Catch a sig_pipe signal, signo = %d, Received Bytes are %ld\n", signo, server_cnt);
    _exit(0);
}

extern void sig_proccess_client(int signo){
    int rc;
    ERR_INFO("Catch a %d signal\n", signo);
    int shmid = getShm();
    struct shm_mem* mem = (struct shm_mem*)shmat(shmid, NULL, 0);
    if(mem != NULL){
        rc = destoryShm(shmid);
        if (rc == 0){
            DEBUG_INFO("Destroy share memory. SHMID is %d\n", shmid);
        } else {
            perror("SHM_DEL");
        }
    } else {
        DEBUG_INFO("Share memory don't exist!\n");
    }

    int semid = getSemSet();
    destorySemSet(semid);
    OUT_INFO("Received Bytes are %ld, Bytes sent are %ld\n", rcv_cnt, sent_cnt);

//    close(sock[0]);
    exit (0);
}