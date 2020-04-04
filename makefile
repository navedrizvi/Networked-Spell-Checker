#Naved Rizvi, CIS 3207, Project 2
CC = gcc
CFLAGS= -Wall

executables = client server

all: client server

server: server.c main.h
	$(CC) $(CFLAGS) -o server server.c -I.

client: client.c main.h
	$(CC) $(CFLAGS) -o client client.c -I.

clean :
	rm $(executables)