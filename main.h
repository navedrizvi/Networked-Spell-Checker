#define DEFAULT_DICTIONARY "dictionary.txt"
#define DEFAULT_PORT 8080
#define MAX_LINE 200
#define BACKLOG 10
#define NUM_CLIENTS 5
#define NUM_WORKERS 6
ssize_t readLine(int fd, void *buffer, size_t n);
void handle_client_request_response(int sockfd);
struct Queue;
void handle_server_request_response(int socket_connection_fd, char **words_array);
void remove_newline_from_string(char *str);
void *worker_thread(void *arg);

void enqueue_client(struct Queue *client_connections_queue, int socket_connection_fd);
int dequeue_client(struct Queue *client_connections_queue);