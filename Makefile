.PHONY:all

//CC = /opt/fsl-networking/QorIQ-SDK-V1.6/sysroots/i686-fslsdk-linux/usr/bin/powerpc-fsl-linux/powerpc-fsl-linux-gcc
CC = gcc
RM = rm
CFLAGS=-Wall

all:server_shm client server server_shm

server_shm: server_shm.o comm.o sem_comm.o signal_handle.o
	$(CC) $^ -o $@

client: client.o signal_handle.o comm.o sem_comm.o
	$(CC) $^ -o $@

server: server.o
	$(CC) $^ -o $@

server_shm.o: server_shm.c
	$(CC) $(CFLAGS) -c $^ -o $@

comm.o: comm.c
	$(CC) $(CFLAGS) -c $^ -o $@

sem_comm.o: sem_comm.c
	$(CC) $(CFLAGS) -c $^ -o $@

signal_handle.o: signal_handle.c
	$(CC) $(CFLAGS) -c $^ -o $@

client.o: client.c
	$(CC) $(CFLAGS) -c $^ -o $@

server.o: server.c
	$(CC) $(CFLAGS) -c $^ -o $@


.PHONY:clean
clean:
	$(RM) -f client server server_shm signal_handle.o client.o comm.o server_shm.o sem_comm.o server.o
