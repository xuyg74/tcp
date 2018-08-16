.PHONY:all
all:server_shm client server server_shm

server_shm:server_shm.c
	gcc -c comm.c
	gcc -c server_shm.c
	gcc -o server_shm server_shm.o comm.o comm.h

client:client.c
	gcc -c signal_handle.c
	gcc -c comm.c
	gcc -c client.c
	gcc -o client client.o signal_handle.o comm.o signal_handle.h

server:server.c
	gcc -o $@ $^

.PHONY:clean
clean:
	rm -f client server signal_handle.o client.o comm.o server_shm.o