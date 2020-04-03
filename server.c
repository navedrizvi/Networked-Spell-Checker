#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "main.h"

int main(int argc, char *argv[])
{
    //Setup network connection
    int socket_desc, new_socket, c;
    struct sockaddr_in server, client;
    char *msg;

    //Create new socket descriptor
    if ((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == 1)
    {
        fprintf(stderr, "Error creating socket: %s", strerror(errno));
        exit(0);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);

    //Bind connects the server's socket address to socket descriptor
    if (bind(socket_desc, &server, sizeof(server)) == -1)
    {
        fprintf(stderr, "Bind error: %s", strerror(errno));
        exit(1);
    }
    printf("Bind done");
    //Listen converts the active socket to LISTENING socket which can accept connections
    listen(socket_desc, 3);
    printf("Waiting for incoming connections...");
    //While waits for and accepts incoming connects
    while (1)
    {
        //Accept incoming connection and create a new CONNECTED descriptor
        c = sizeof(struct sockaddr_in);
        new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
        if (new_socket == -1)
        {
            fprintf(stderr, "Error: Accept Failed:  %s", strerror(errno));
            exit(1);
        }
        printf("Connection accepted");
    }
}
