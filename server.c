#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include "main.h"
#include "spell_checking.c"
#include "utility.c"

sbuf_t sbuf;

int main(int argc, char *argv[])
{
    int socket_fd, connection_fd, client_len;
    struct sockaddr_in servaddr, cliaddr;
    int port_number;
    char *dict_file;
    struct WordsArray words_array;
    pthread_t tid;

    pthread_mutex_t lock;
    int rc = pthread_mutex_init(&lock, NULL);

    //Check user arguments and assign accordingly
    port_number = DEFAULT_PORT;
    if (argv[1] != NULL)
    {
        dict_file = argv[1];
        if (argv[2] != NULL)
        {
            port_number = atoi(argv[2]);
        }
    }
    else
    {
        dict_file = DEFAULT_DICTIONARY;
    }

    words_array = create_word_array_from_file(dict_file);

    //===============Establish network connection ==================//
    //Create new socket descriptor
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 1)
    {
        unix_error("Socket Creation Failed");
    }
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port_number);

    //Bind connects the server's socket address to socket descriptor
    if (bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        unix_error("Bind Failed");
    }
    puts("Bind done");
    //Listen converts the ACTIVE socket to LISTENING socket which can accept connections
    if ((listen(socket_fd, BACKLOG)) != 0)
    {
        unix_error("Listen Failed");
    }

    printf("Listenting on port: %d\n", port_number);

    //Create worker threads
    sbuf_init(&sbuf, SBUFSIZE);
    for (int i = 0; i < NTHREADS; i++)
    {
        Pthread_create(&tid, NULL, thread, NULL);
    }

    //===============Establish network connection end==================//
    //While waits for and accepts incoming connects
    while (1)
    {
        client_len = sizeof(struct sockaddr_in);
        printf("Waiting for incoming connections...\n\n");
        //Accept incoming connection and create a new CONNECTED descriptor
        if ((connection_fd = accept(socket_fd, (struct sockaddr *)&cliaddr, (socklen_t *)&client_len)) == -1)
        {
            unix_error("Accept Failed");
        }
        sbuf_insert(&sbuf, connection_fd);

        // puts("Connection accepted");
        // char msg[] = "\nWelcome to spell checker!\nTo spell check, type the word and press Enter key\nTo exit, press Escape key (ESC) and press Enter key\n\n";
        // write(connection_fd, msg, sizeof(msg));
        // handle_server_request_response(connection_fd, words_array);

        // puts("Connection closed...\n");
        // close(connection_fd);
    }

    //Destroy mutex
    pthread_mutex_destroy(&lock);

    return 0;
}

void *thread(void *arg)
{
    printf("here");
    Pthread_detach(pthread_self());
    while (1)
    {
        int connection_fd = sbuf_remove(&sbuf);
        puts("Connection accepted");
        char msg[] = "\nWelcome to spell checker!\nTo spell check, type the word and press Enter key\nTo exit, press Escape key (ESC) and press Enter key\n\n";
        write(connection_fd, msg, sizeof(msg));
        // handle_server_request_response(connection_fd, words_array);

        puts("Connection closed...\n");
        close(connection_fd);
    }
}

void handle_server_request_response(int socket_connection_fd, struct WordsArray words_array)
{
    char buff[MAX_LINE];
    int word_found_index;

    while (1)
    {
        bzero(buff, MAX_LINE);

        //Read the message from client into buffer
        read(socket_connection_fd, buff, sizeof(buff));
        //if user pressed escape exit chat and close socket connection.
        if ((int)buff[0] == 27)
        {
            printf("Server exiting...\n");
            break;
        }
        remove_newline_from_string(buff);
        printf("just read: %s\n", buff);
        word_found_index = linear_search(buff, words_array);
        bzero(buff, MAX_LINE);
        // copy server message in the buffer
        if (word_found_index == -1)
        {
            strcpy(buff, "MISSPELLED\n\0");
        }
        else
        {
            strcpy(buff, "OK\n\0");
        }

        write(socket_connection_fd, buff, sizeof(buff));
    }
}
