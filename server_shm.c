#include "comm.h"
#include <unistd.h>

#include <sys/wait.h>
#include "sem_comm.h"

int main()
{
	int shmid = creatShm();
	char* mem = (char*)shmat(shmid, NULL, 0);

	int semid = creatSemSet(1);
	initSem(semid,0);
#if 1
	int i = 1;
	while(1)
	{
		sleep(2);
		P(semid,0);
		if(mem[0] == 0x00){
			mem[0] = 0x1;
			mem[i++] = 'A';
			i %= (SIZE-1);
			mem[i] = '\0';
		} else {
			printf("The data is not updated!\n");
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
	shmdt(mem);
	destoryShm(shmid);
	return 0;
}