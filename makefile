#Naved Rizvi, CIS 3207, Project 2
CC = gcc
CFLAGS= -Wall

executables = client server multi_client

all: client server multi_client

server: server.c utility.c main.h
	$(CC) $(CFLAGS) -o server server.c -I. -pthread

client: client.c utility.c main.h
	$(CC) $(CFLAGS) -o client client.c -I. -pthread

multi_client: multi_client.c utility.c main.h
	$(CC) $(CFLAGS) -o multi_client multi_client.c -I. -pthread

clean :
	rm $(executables)