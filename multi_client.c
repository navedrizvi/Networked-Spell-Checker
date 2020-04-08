#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "utility.c"
#include "main.h"

int main(int argc, char *argv[])
{
    pthread_t thread_pool[NUM_WORKERS];

    for (int i = 0; i < NUM_CLIENT_THREADS; i++)
        Pthread_create(&thread_pool[i], NULL, client_thread, (void *)argv);
    sleep(20);
    for (int i = 0; i < NUM_CLIENT_THREADS; i++)
    {
        if (pthread_join(&thread_pool[i], NULL) != 0)
        {
            fprintf(stderr, "Thread join error\n");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

void *client_thread(void *arg)
{
    int socket_fd;
    struct sockaddr_in servaddr;
    char *host;
    int port_number;

    char **argv = (char **)arg;

    if (argv[1] == NULL)
    {
        fprintf(stderr, "usage: %s host [server_port_n]\n", argv[0]);
        exit(1);
    }
    host = argv[1];
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        unix_error("Socket Creation Failed");
    }

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(host);

    //Set port for server if specified
    if (argv[2] != NULL)
    {
        port_number = atoi(argv[2]);
    }
    else
    {
        port_number = DEFAULT_PORT;
    }

    servaddr.sin_port = htons(port_number);
    //Call connect to establish connection with server (servaddr)
    if (connect(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)

    {
        unix_error("Connection with the server failed..");
    }
    printf("Connected to server..\n");
    printf("Response from server:\n");

    char message[MAX_LINE];
    read(socket_fd, message, sizeof(message));
    printf("%s", message);
    handle_client_request_response(socket_fd);

    close(socket_fd);
}

void handle_client_request_response(int socket_fd)
{
    char buff[MAX_LINE];
    int n;
    while (1)
    {
        //Get client input
        bzero(buff, sizeof(buff));

        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;
        //Send client input
        write(socket_fd, buff, sizeof(buff));
        if ((int)buff[0] == 27)
        {
            printf("Client exiting...\n");
            break;
        }

        bzero(buff, sizeof(buff));

        //Read response from server
        read(socket_fd, buff, sizeof(buff));
        printf("Response form server : %s", buff);
    }
}
