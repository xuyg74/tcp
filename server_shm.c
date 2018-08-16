#include"comm.h"
#include<unistd.h>

int main()
{
	int shmid = creatShm();
	char* mem = (char*)shmat(shmid, NULL, 0);
#if 1
	int i = 0;
	while(1)
	{
		usleep(100);
		mem[i++] = 'A';
		i %= (SIZE-1);
		mem[i] = '\0';
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