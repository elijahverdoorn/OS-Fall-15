current: server

server.o: server.c
	gcc -c server.c

client.o: client.c
	gcc -c client.c

server: server.o server.c
	gcc -pthread -o server server.o

client: client.o client.c
	gcc -o client client.o