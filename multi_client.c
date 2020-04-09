#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "utility.c"
#include "main.h"

pthread_mutex_t word_queue_lock;

pthread_cond_t word_queue_has_item;

struct ThreadArguments
{
    struct WordQueue *words_queue;
    int server_socket_decriptor;
};

struct ThreadArguments *allocate_thread_arguments(struct WordQueue *words_queue, int server_socket_descriptor)
{
    struct ThreadArguments *args = (struct ThreadArguments *)malloc(sizeof(struct ThreadArguments));
    args->words_queue = words_queue;
    args->server_socket_decriptor = server_socket_descriptor;

    return args;
}

int main(int argc, char *argv[])
{
    char *host;
    char *input_file_name;
    struct WordQueue *words_queue;
    int port_number;
    int socket_fd;
    pthread_t thread_pool[NUM_WORKERS];
    struct sockaddr_in servaddr;
    struct ThreadArguments *thread_args;
    char initial_response[MAX_LINE];

    //Initialize mutex
    if (pthread_mutex_init(&word_queue_lock, NULL) != 0)
    {
        unix_error("Mutex initilization error for log queue");
    }

    //Parse user arguments and assign parameters accordingly
    if (argv[1] == NULL)
    {
        fprintf(stderr, "usage: %s host [server_port_n] [words_file_name]\n", argv[0]);
        exit(1);
    }

    host = argv[1];

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        unix_error("Socket Creation Failed");
    }

    port_number = DEFAULT_PORT;
    input_file_name = DEFAULT_CLIENT_INPUT;

    //Set port for server and input file if specified
    if (argv[2] != NULL)
    {
        port_number = atoi(argv[2]);
        if (argv[3] != NULL)
        {
            input_file_name = argv[3];
        }
    }

    //Create a array of words to spell check on
    words_queue = create_words_queue_from_file(input_file_name);

    //=============== Establish network connection ==================//
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(host);
    servaddr.sin_port = htons(port_number);

    //Call connect to establish connection with server (servaddr)
    if (connect(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)

    {
        unix_error("Connection with the server failed..");
    }
    printf("Connected to server.. \n");
    printf("Response from server:\n");
    read(socket_fd, initial_response, sizeof(initial_response));
    printf("%s", initial_response);
    //=============== Establish network connection end ==================/

    // Create worker threads
    thread_args = allocate_thread_arguments(words_queue, socket_fd);

    for (int i = 0; i < NUM_CLIENT_THREADS; i++)
        Pthread_create(&thread_pool[i], NULL, client_thread, (void *)thread_args);

    //Signal waiting threads that there are items in queue
    pthread_cond_signal(&word_queue_has_item);

    for (int i = 0; i < NUM_CLIENT_THREADS; i++)
    {
        if (pthread_join(thread_pool[i], NULL) != 0)
        {
            unix_error("Thread join error");
        }
    }

    //Deallocate mutex and condition variable
    pthread_mutex_destroy(&word_queue_lock);
    pthread_cond_destroy(&word_queue_has_item);

    //Free allocated datastructures
    free(thread_args);
    free(words_queue);

    //Close socket connection
    close(socket_fd);

    return EXIT_SUCCESS;
}

char *dequeue_word_thread_safe(struct WordQueue *word_queue)
{
    char *first_word;

    pthread_mutex_lock(&word_queue_lock);

    while (word_queue_is_empty(word_queue))                        //while buffer is empty, condition wait till its not empty
        pthread_cond_wait(&word_queue_has_item, &word_queue_lock); // wait till queue has some item

    //Remove element
    first_word = dequeue_word(word_queue);

    pthread_mutex_unlock(&word_queue_lock);

    return first_word;
}

void *client_thread(void *arg)
{
    struct ThreadArguments *thread_args = (struct ThreadArguments *)arg;
    char *word;
    while (!word_queue_is_empty(thread_args->words_queue))
    {
        word = dequeue_word_thread_safe(thread_args->words_queue);
        handle_multithreaded_client_request_response(thread_args->server_socket_decriptor, word);

        //If still not empty then signal other threads
        if (!word_queue_is_empty(thread_args->words_queue))
        {
            pthread_cond_signal(&word_queue_has_item);
        }
    }

    return NULL;
}

void handle_multithreaded_client_request_response(int socket_fd, char *word_to_check)
{
    char buff[MAX_LINE];
    bzero(buff, MAX_LINE);

    //Send client input
    write(socket_fd, word_to_check, (strlen(word_to_check) + 2));

    //If client uses Escape key
    if ((int)word_to_check[0] == 27)
    {
        printf("Client exiting...\n");
        return;
    }

    //Read response from server
    read(socket_fd, buff, sizeof(buff));
    printf("Response from server : %s", buff);
}
