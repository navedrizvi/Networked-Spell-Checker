#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "main.h"
#include "spell_checker.c"

int main(int argc, char *argv[])
{
    //Setup network connection
    int socket_fd, connection_fd, c;
    struct sockaddr_in server, client;
    char *msg;
    ssize_t bytes_read;
    char line[MAX_LINE];
    uint8_t buff[MAX_LINE + 1];

    struct WordsArray words_array = create_word_array_from_file(DEFAULT_DICTIONARY);
    int word_found_index;

    //Create new socket descriptor
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 1)
    {
        puts("Error: Socket creation failed");
        exit(0);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(DEFAULT_PORT);

    //Bind connects the server's socket address to socket descriptor
    if (bind(socket_fd, &server, sizeof(server)) == -1)
    {
        puts("Error: Bind failed");
        exit(0);
    }
    puts("Bind done");
    //Listen converts the ACTIVE socket to LISTENING socket which can accept connections
    listen(socket_fd, BACKLOG);
    puts("Waiting for incoming connections...");
    //While waits for and accepts incoming connects
    while (1)
    {
        //Accept incoming connection and create a new CONNECTED descriptor
        c = sizeof(struct sockaddr_in);
        connection_fd = accept(socket_fd, (struct sockaddr *)&client, (socklen_t *)&c);
        if (connection_fd == -1)
        {
            puts("Error: Accept failed");
            exit(0);
        }
        puts("Connection accepted");

        while ((bytes_read = readLine(connection_fd, line, MAX_LINE - 1)) > 0)
        {
            printf("just read: %s", line);

            if ((word_found_index = linear_search(line, words_array)) == -1)
            {
                msg = "MISSPELLED\n";
            }
            else
            {
                msg = "OK\n";
            }

            write(connection_fd, (char *)msg, strlen((char *)msg));
        }
    }
    puts("Connection closed...\n");
    close(connection_fd);
    return 0;
}

ssize_t readLine(int fd, void *buffer, size_t n)
{
    ssize_t numRead; /* # of bytes fetched by last read() */
    size_t totRead;  /* Total bytes read so far */
    char *buf;
    char ch;

    if (n <= 0 || buffer == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    buf = buffer; /* No pointer arithmetic on "void *" */

    totRead = 0;
    for (;;)
    {
        numRead = read(fd, &ch, 1);

        if (ch == '\n')
            break;
        if (numRead == -1)
        {
            if (errno == EINTR) /* Interrupted --> restart read() */
                continue;
            else
                return -1; /* Some other error */
        }
        else if (numRead == 0)
        {                     /* EOF */
            if (totRead == 0) /* No bytes read; return 0 */
                return 0;
            else /* Some bytes read; add '\0' */
                break;
        }
        else
        { /* 'numRead' must be 1 if we get here */
            if (totRead < n - 1)
            { /* Discard > (n - 1) bytes */
                totRead++;
                *buf++ = ch;
            }

            // if (ch == '\n')
            //     break;
        }
    }

    *buf = '\0';
    return totRead;
}