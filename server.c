#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include "main.h"
#include "utility.c"

//Mutexes
pthread_mutex_t client_connections_queue_lock;
pthread_mutex_t log_queue_lock;

//Condition variables
pthread_cond_t client_connections_queue_has_item;
pthread_cond_t client_connections_queue_has_empty_space;
pthread_cond_t log_queue_has_item;
pthread_cond_t log_queue_has_empty_space;

struct ThreadArguments
{
    char **words_array;
    struct ClientQueue *client_connections_queue;
    struct LogQueue *log_queue;
};

struct ThreadArguments *allocate_thread_arguments(char **words_array, struct ClientQueue *connections_queue, struct LogQueue *log_queue)
{
    struct ThreadArguments *args = (struct ThreadArguments *)malloc(sizeof(struct ThreadArguments));
    args->words_array = words_array;
    args->client_connections_queue = connections_queue;
    args->log_queue = log_queue;

    return args;
}

int main(int argc, char *argv[])
{
    int socket_fd, connection_fd, client_len;
    struct sockaddr_in servaddr, cliaddr;
    int port_number;
    char *dict_file_name;
    char **words_array;
    struct ClientQueue *client_connections_queue = allocate_client_queue_with_capacity(NUM_CLIENTS);
    struct LogQueue *log_queue = allocate_log_queue(LOG_CAPACITY);
    struct ThreadArguments *thread_args;
    pthread_t worker_thread_pool[NUM_WORKERS];
    pthread_t logger_pool[NUM_LOGGERS];

    //Initialize mutexes
    if (pthread_mutex_init(&client_connections_queue_lock, NULL) != 0)
    {
        unix_error("Mutex initilization error for worker queue");
    }
    if (pthread_mutex_init(&log_queue_lock, NULL) != 0)
    {
        unix_error("Mutex initilization error for log queue");
    }

    //Initialize condition variables
    if (pthread_cond_init(&client_connections_queue_has_empty_space, NULL) != 0)
    {
        unix_error("Condition variable initilization error for full client queue");
    }
    if (pthread_cond_init(&client_connections_queue_has_item, NULL) != 0)
    {
        unix_error("Condition variable initilization error for empty client queue");
    }
    if (pthread_cond_init(&log_queue_has_empty_space, NULL) != 0)
    {
        unix_error("Condition variable initilization error for full log queue");
    }
    if (pthread_cond_init(&log_queue_has_item, NULL) != 0)
    {
        unix_error("Condition variable initilization error for empty log queue");
    }

    //Parse user arguments and assign parameters accordingly
    port_number = DEFAULT_PORT;
    if (argv[1] != NULL)
    {
        dict_file_name = argv[1];
        if (argv[2] != NULL)
        {
            port_number = atoi(argv[2]);
        }
    }
    else
    {
        dict_file_name = DEFAULT_DICTIONARY;
    }

    //Create a array of words for spell checking function
    words_array = create_word_array_from_file(dict_file_name);

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

    //===============Establish network connection end==================//

    // Create worker threads
    thread_args = allocate_thread_arguments(words_array, client_connections_queue, log_queue);
    for (int i = 0; i < NUM_WORKERS; i++)
    {
        Pthread_create(&worker_thread_pool[i], NULL, worker_thread, (void *)thread_args);
    }

    for (int i = 0; i < NUM_LOGGERS; i++)
    {
        Pthread_create(&logger_pool[i], NULL, logger_thread, (void *)thread_args);
    }

    printf("Waiting for incoming connections...\n\n");
    //While waits for and accepts incoming connects
    while (1)
    {
        client_len = sizeof(struct sockaddr_in);
        //Accept incoming connection and create a new CONNECTED descriptor
        if ((connection_fd = accept(socket_fd, (struct sockaddr *)&cliaddr, (socklen_t *)&client_len)) == -1)
        {
            unix_error("Accept Failed");
        }

        if (!queue_is_full(client_connections_queue)) //check if queue is not full (NUM_CLIENTS is queue_capacity+1)
        {
            puts("New client accepted");
            char msg[] = "\nWelcome to spell checker!\nTo spell check, type the word and press Enter key\nTo exit, press Escape key (ESC) and press Enter key\n\nEnter word to spell check: ";
            write(connection_fd, msg, sizeof(msg));
            enqueue_client(client_connections_queue, connection_fd);
        }
        else
        {
            char msg[] = "\nSorry, the server is overloaded, please try later\n\n";
            write(connection_fd, msg, sizeof(msg));
        }
    }

    close(socket_fd);

    //Deallocate mutexes
    pthread_mutex_destroy(&client_connections_queue_lock);
    pthread_mutex_destroy(&log_queue_lock);

    //Deallocate condition variables
    pthread_cond_destroy(&client_connections_queue_has_item);
    pthread_cond_destroy(&client_connections_queue_has_empty_space);
    pthread_cond_destroy(&log_queue_has_item);
    pthread_cond_destroy(&log_queue_has_empty_space);

    //Free allocated datastructures
    free(client_connections_queue);
    free(log_queue);
    free(thread_args);
    free(words_array);

    return 0;
}

void enqueue_client(struct ClientQueue *client_connections_queue, int socket_connection_fd)
{
    pthread_mutex_lock(&client_connections_queue_lock);

    while (queue_is_full(client_connections_queue))                                                   //while buffer is full, condition wait till its not full
        pthread_cond_wait(&client_connections_queue_has_empty_space, &client_connections_queue_lock); // wait till queue has some empty space

    //Insert client
    enqueue(client_connections_queue, socket_connection_fd);

    //Signal waiting threads
    pthread_cond_signal(&client_connections_queue_has_item);

    pthread_mutex_unlock(&client_connections_queue_lock);
}

int dequeue_client(struct ClientQueue *client_connections_queue)
{
    int client_socket_fd;
    pthread_mutex_lock(&client_connections_queue_lock);

    while (queue_is_empty(client_connections_queue))                                           //while buffer is empty, condition wait till its not empty
        pthread_cond_wait(&client_connections_queue_has_item, &client_connections_queue_lock); // wait till queue has some item

    //Remove element
    client_socket_fd = dequeue(client_connections_queue);

    //Signal waiting threads to notify theres empty spot in queue
    pthread_cond_signal(&client_connections_queue_has_empty_space);

    pthread_mutex_unlock(&client_connections_queue_lock);

    return client_socket_fd;
}

void enqueue_log_event(struct LogQueue *log_queue, struct LogNode *log_record)
{
    pthread_mutex_lock(&log_queue_lock);

    while (log_queue_is_full(log_queue))                                //while buffer is full, condition wait till its not full
        pthread_cond_wait(&log_queue_has_empty_space, &log_queue_lock); // wait till queue has some empty space

    //Insert log
    enqueue_log(log_queue, log_record);

    //Signal waiting threads
    pthread_cond_signal(&log_queue_has_item);

    pthread_mutex_unlock(&log_queue_lock);
}

struct LogNode *dequeue_log_event(struct LogQueue *log_queue)
{
    struct LogNode *log_record;
    pthread_mutex_lock(&log_queue_lock);

    // while (log_queue_is_empty(log_queue))                        //while buffer is empty, condition wait till its not empty
    //     pthread_cond_wait(&log_queue_has_item, &log_queue_lock); // wait till queue has some item

    // //Remove element
    log_record = dequeue_log(log_queue);

    // //Signal waiting threads to notify theres empty spot in queue
    // pthread_cond_signal(&log_queue_has_empty_space);

    pthread_mutex_unlock(&log_queue_lock);

    return log_record;
}

void *worker_thread(void *arg)
{
    int connection_fd;

    struct ThreadArguments *thread_args = (struct ThreadArguments *)arg;

    while (1)
    {
        while (!queue_is_empty(thread_args->client_connections_queue))
        {
            connection_fd = dequeue_client(thread_args->client_connections_queue);

            handle_server_request_response(connection_fd, thread_args->words_array, thread_args->log_queue);

            puts("Connection closed...\n");
            close(connection_fd);
        }
    }
}

void *logger_thread(void *arg)
{
    FILE *log_ptr;
    struct LogNode *log_record;
    struct ThreadArguments *thread_args = (struct ThreadArguments *)arg;

    while (1)
    {
        while (!log_queue_is_empty(thread_args->log_queue))
        {
            //Dequeue event to write to file in a thead safe manner
            // log_record = dequeue_log(thread_args->log_queue);
            pthread_mutex_lock(&log_queue_lock);

            // while (log_queue_is_empty(log_queue))                        //while buffer is empty, condition wait till its not empty
            //     pthread_cond_wait(&log_queue_has_item, &log_queue_lock); // wait till queue has some item

            // //Remove element
            log_record = dequeue_log(thread_args->log_queue);

            // //Signal waiting threads to notify theres empty spot in queue
            // pthread_cond_signal(&log_queue_has_empty_space);

            if ((log_ptr = fopen(DEFAULT_LOG_FILE, "a")) == NULL) //open log file in append mode
            {
                unix_error("Error opening log file");
            }
            if (log_record->correctness != -1)
            {
                fprintf(log_ptr, "%s - OK.\n", log_record->word);
            }
            else
            {
                fprintf(log_ptr, "%s - MISPELLED.\n", log_record->word);
            }
            fclose(log_ptr);
            pthread_mutex_unlock(&log_queue_lock);
        }
    }
}

void handle_server_request_response(int socket_connection_fd, char **words_array, struct LogQueue *log_queue)
{
    char buff[MAX_LINE];
    int word_found_index;
    struct LogNode *log_event;

    while (1) //Read while there are word left to read
    {
        bzero(buff, MAX_LINE);

        //Read the message from client into buffer
        read(socket_connection_fd, buff, sizeof(buff));

        //If user pressed escape exit chat and close socket connection.
        if ((int)buff[0] == 27)
        {
            printf("Server exiting...\n");
            break;
        }

        //Client servicing
        remove_newline_from_string(buff);
        printf("just read: %s\n", buff);

        if ((word_found_index = binary_search(buff, words_array)) == -1) //double check when binary search fails
        {
            word_found_index = linear_search(buff, words_array);
        }

        //Create log event and write to log file
        log_event = create_new_log(buff, word_found_index);
        enqueue_log_event(log_queue, log_event);

        //Copy server message in the buffer
        if (word_found_index == -1)
        {
            strcat(buff, " - MISSPELLED\n\n");
        }
        else
        {
            strcat(buff, " - OK\n\n");
        }

        strcat(buff, "Enter word to spell check: ");

        //Send response to client
        write(socket_connection_fd, buff, sizeof(buff));
    }
}
