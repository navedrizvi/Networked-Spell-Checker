#define DEFAULT_DICTIONARY "dictionary.txt"
#define NUM_WORKERS 0
#define DEFAULT_PORT 22222
#define MAX_LINE 200
#define BACKLOG 10
ssize_t readLine(int fd, void *buffer, size_t n);
void handle_client_request_response(int sockfd);
struct WordsArray;

void handle_server_request_response(int socket_connection_fd, struct WordsArray words_array);
void remove_newline_from_string(char *str);