all: client server

clean:
	rm client server

client: client.c
	gcc -std=c99 -Wall -pedantic -o client client.c

server: server.c
	gcc -std=c99 -Wall -pedantic -o server server.c	
