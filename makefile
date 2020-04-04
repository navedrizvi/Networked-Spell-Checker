#Naved Rizvi, CIS 3207, Project 2
CC = gcc
CFLAGS= -Wall

executables = web_client web_server server

all: web_server web_client server

server: server.c main.h
	$(CC) $(CFLAGS) -o server server.c -I.

web_server: web_server.c main.h
	$(CC) $(CFLAGS) -o web_server web_server.c -I.

web_client: web_client.c main.h
	$(CC) $(CFLAGS) -o web_client web_client.c -I.

clean :
	rm $(executables)