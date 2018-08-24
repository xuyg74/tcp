#include"comm.h"
#include<unistd.h>
#include<string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include<sys/wait.h>
#include"sem_comm.h"
#include <errno.h>

int main()
{
	//Create Share Memory
	int shmid = creatShm();
	char* mem = (char*)shmat(shmid, NULL, 0);
	if(NULL == mem){
		printf("Failed to create SHM!\n");
	}

	//Create Semphore
	int semid = creatSemSet(1);
	initSem(semid,0);
	sleep(10);
#if 0
	int i = 1;
	while(1)
	{
		usleep(1000);
		P(semid,0);
		if(mem[0] == 0x00){
			mem[0] = 0x1;
			mem[i++] = 'A';
			i %= (SIZE-1);
			mem[i] = '\0';
		}
		V(semid,0);
	}
#endif

#if 0
	while(1){
		sleep(1);
		mem[0] = 'A';
		mem[1] = 'B';
		mem[2] = 'C';
		mem[3] = 'D';
		mem[4] = '\0';
	}
#endif

#if 1
	//Open a file
	char *filename = "/media/sf_share/exam/example";
	int fp = open(filename,O_RDWR,S_IWUSR);
	if(-1 == fp){
		printf("The file %s can't be open.\n", filename);
	} else {
		printf("The file %s has been open.\n", filename);
	}

	while(1){
		usleep(10000);
		P(semid,0);
		if(mem[0] == 0x00){
			bzero(mem, SIZE);
			int num_bytes = read(fp, mem+1, SIZE-8);  // Read the data from the file
			if(num_bytes==0){
				printf("The file %s has been finished.\n", filename);
				close(fp);
				V(semid,0);
				break;
			}else if(num_bytes == -1){
				printf("The file %s has error to be read. errno: %d!\n", filename, errno);
				close(fp);
				V(semid,0);
				break;
			} else {
				printf("%d bytes has been read!\n", num_bytes);
				fflush(stdout);
				mem[0] = 0x1;
				V(semid,0);
			}
		}
	}

#endif
	shmdt(mem);
	destoryShm(shmid);
	return 0;
}