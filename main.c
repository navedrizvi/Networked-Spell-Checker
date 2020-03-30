#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    //Set up connection to client
    int socket_desc, new_socket, c;
    struct sockaddr_in server, client;
    char *msg;

    if ((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == 1)
    {
        fprintf(stderr, "Error creating socket: %s", strerror(errno));
        exit(0);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(portnumber);
}
