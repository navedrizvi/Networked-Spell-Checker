#Naved Rizvi, CIS 3207, Project 2
CC = gcc
CFLAGS= -Wall

objects = server.o

server: server.c main.h
	$(CC) $(CFLAGS) -c server.c

clean :
	rm $(objects)