#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include "main.h"
#include "spell_checker.c"

int main(int argc, char *argv[])
{
    //Setup network connection
    int socket_fd, connection_fd, client_len;
    struct sockaddr_in servaddr, cliaddr;
    int port_number;
    struct WordsArray words_array = create_word_array_from_file(DEFAULT_DICTIONARY);

    //Check user arguments and assign accordingly
    if (argv[1] != NULL)
    {
        port_number = atoi(argv[1]);
    }
    else
    {
        port_number = DEFAULT_PORT;
    }
    //Create new socket descriptor
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 1)
    {
        puts("Error: Socket creation failed");
        exit(0);
    }
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port_number);

    //Bind connects the server's socket address to socket descriptor
    if (bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        puts("Error: Bind failed");
        exit(0);
    }
    puts("Bind done");
    //Listen converts the ACTIVE socket to LISTENING socket which can accept connections
    if ((listen(socket_fd, BACKLOG)) != 0)
    {
        printf("Listen failed...\n");
        exit(0);
    }
    printf("Listenting on port: %d. Waiting for incoming connections...\n\n", port_number);

    //While waits for and accepts incoming connects
    client_len = sizeof(struct sockaddr_in);
    while (1)
    {
        //Accept incoming connection and create a new CONNECTED descriptor
        connection_fd = accept(socket_fd, (struct sockaddr *)&cliaddr, (socklen_t *)&client_len);
        if (connection_fd == -1)
        {
            puts("Error: Accept connection failed");
            exit(0);
        }
        puts("Connection accepted");
        char msg[] = "\nWelcome to spell checker!\nTo spell check, type the word and press Enter key\nTo exit, press Escape key (ESC) and press Enter key\n\n";
        write(connection_fd, msg, sizeof(msg));
        handle_server_request_response(connection_fd, words_array);

        puts("Connection closed...\n");
        close(connection_fd);
    }

    return 0;
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

void remove_newline_from_string(char *str)
{
    str[strlen(str) - 1] = '\0';
}
