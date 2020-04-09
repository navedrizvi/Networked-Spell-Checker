#define DEFAULT_DICTIONARY "dictionary.txt"
#define DEFAULT_PORT 8080
#define MAX_LINE 400
#define BACKLOG 10
#define NUM_CLIENTS 3
#define NUM_WORKERS 6
#define NUM_LOGGERS 1
#define LOG_CAPACITY 6
#define DEFAULT_LOG_FILE "log.txt"
#define NUM_CLIENT_THREADS 4
#define DEFAULT_CLIENT_INPUT "client_input.txt"

struct ClientQueue;
struct LogQueue;
ssize_t readLine(int fd, void *buffer, size_t n);
void handle_client_request_response(int sockfd);
void handle_multithreaded_client_request_response(int socket_fd, char *word_to_check);
void handle_server_request_response(int socket_connection_fd, char **words_array, struct LogQueue *log_queue);
void remove_newline_from_string(char *str);
void *worker_thread(void *arg);
void *logger_thread(void *arg);
void *client_thread(void *arg);
void enqueue_client_thread_safe(struct ClientQueue *client_connections_queue, int socket_connection_fd);
int dequeue_client_thread_safe(struct ClientQueue *client_connections_queue);