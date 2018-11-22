ver = debug

ifeq ($(ver), debug)
	CFLAGS=-Wall -Ddebug
else
	CFLAGS=-Wall
endif

.PHONY:all

//CC = /opt/fsl-networking/QorIQ-SDK-V1.6/sysroots/i686-fslsdk-linux/usr/bin/powerpc-fsl-linux/powerpc-fsl-linux-gcc
CC = gcc
RM = rm
OBJ_DIR = ./obj
BIN_DIR = ./bin

all:client_all server mymv

client_all: client_all.o comm.o sem_comm.o pcap.o pthread_tools.o readconf.o signal_handle.o
	$(CC) $^ -o $@ -lpthread -lpcap

server: server.o readconf.o comm.o sem_comm.o signal_handle.o
	$(CC) $^ -o $@ -lpthread

server_shm.o: server_shm.c
	$(CC) $(CFLAGS) -c $^ -o $@

comm.o: comm.c
	$(CC) $(CFLAGS) -c $^ -o $@

sem_comm.o: sem_comm.c
	$(CC) $(CFLAGS) -c $^ -o $@

signal_handle.o: signal_handle.c
	$(CC) $(CFLAGS) -c $^ -o $@

client_all.o: client_all.c
	$(CC) $(CFLAGS) -c $^ -o $@

server.o: server.c
	$(CC) $(CFLAGS) -c $^ -o $@

pcap.o: pcap.c
	$(CC) $(CFLAGS) -c $^ -o $@

pthread_tools.o: pthread_tools.c
	$(CC) $(CFLAGS) -c $^ -o $@

readconf.o: readconf.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY:clean
clean:
	$(RM) -rf $(OBJ_DIR) $(BIN_DIR)

mymv:
	mkdir -p $(OBJ_DIR) $(BIN_DIR)
	mv -f *.o $(OBJ_DIR)/
	mv -f client_all $(BIN_DIR)/
	mv -f server $(BIN_DIR)/