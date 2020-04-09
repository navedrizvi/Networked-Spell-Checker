#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"

/********************************* FIFO Queues for client servicing and logging **************************************/
struct ClientQueue
{
    int front, rear, size;
    unsigned capacity;
    int *array;
};

struct LogNode
{
    char *word;
    int correctness; //case: 0 = spelled incorrecly, 1 = spelled correctly
    struct LogNode *next;
};

struct LogQueue
{
    struct LogNode *head;
    struct LogNode *tail;
    int size;
    int capacity;
};

struct WordNode
{
    char *word;
    struct WordNode *next;
};

struct WordQueue
{
    struct WordNode *head;
    struct WordNode *tail;
    int size;
    int capacity;
};

int enqueue_client(struct ClientQueue *queue, int item)
{
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;

    return 0;
}

int dequeue_client(struct ClientQueue *queue)
{
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;

    return item;
}

void enqueue_log(struct LogQueue *q, struct LogNode *n)
{
    if (q->tail == NULL)
    {
        q->head = q->tail = n;
        q->size += 1;
        return;
    }

    q->tail->next = n;
    q->tail = n;
    q->size += 1;
}

struct LogNode *dequeue_log(struct LogQueue *q)
{
    struct LogNode *first_log = q->head;
    q->head = q->head->next;
    q->size -= 1;
    if (q->head == NULL)
    {
        q->tail = NULL;
    }

    return first_log;
}

void enqueue_word(struct WordQueue *q, struct WordNode *n)
{
    if (q->tail == NULL)
    {
        q->head = q->tail = n;
        q->size += 1;
        return;
    }

    q->tail->next = n;
    q->tail = n;
    q->size += 1;
}

char *dequeue_word(struct WordQueue *q)
{
    char *first_word = q->head->word;
    q->head = q->head->next;
    q->size -= 1;
    if (q->head == NULL)
    {
        q->tail = NULL;
    }

    return first_word;
}

//Create new log
struct LogNode *create_new_log(char *word, int correctness)
{
    struct LogNode *temp = (struct LogNode *)malloc(sizeof(struct LogNode));
    temp->word = (char *)malloc(sizeof(char) * strlen(word));
    strcpy(temp->word, word);
    temp->correctness = correctness;
    temp->next = NULL;

    return temp;
}

//Create new word node
struct WordNode *create_new_word_node(char *word)
{
    struct WordNode *temp = (struct WordNode *)malloc(sizeof(struct WordNode));
    temp->word = (char *)malloc(sizeof(char) * strlen(word));
    strcpy(temp->word, word);
    temp->next = NULL;

    return temp;
}

//Create new log queue
struct LogQueue *allocate_log_queue_with_capacity(unsigned capacity)
{
    struct LogQueue *q = (struct LogQueue *)malloc(sizeof(struct LogQueue));
    q->head = q->tail = NULL;
    q->size = 0;
    q->capacity = capacity;

    return q;
}

//Create new word queue
struct WordQueue *allocate_word_queue_with_capacity(unsigned capacity)
{
    struct WordQueue *q = (struct WordQueue *)malloc(sizeof(struct WordQueue));
    q->head = q->tail = NULL;
    q->capacity = capacity;

    return q;
}

struct ClientQueue *allocate_client_queue_with_capacity(unsigned capacity)
{
    struct ClientQueue *queue = (struct ClientQueue *)malloc(sizeof(struct ClientQueue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->array = (int *)malloc(queue->capacity * sizeof(int));

    return queue;
}

int queue_is_full(struct ClientQueue *queue)
{
    return (queue->size == queue->capacity);
}

int log_queue_is_full(struct LogQueue *queue)
{
    return (queue->size == queue->capacity);
}

int queue_is_empty(struct ClientQueue *queue)
{
    return (queue->size <= 0);
}

int log_queue_is_empty(struct LogQueue *queue)
{
    return (queue->size <= 0);
}

int word_queue_is_empty(struct WordQueue *queue)
{
    return (queue->size <= 0);
}

/********************************* Spell Checking Helpers **************************************/

//Perform binary search using strcmp - return -1 if not found, else return the index
int binary_search(char *word, char **words_array)
{
    int length = 0;
    char **ptr = words_array;
    while (*ptr != NULL)
    {
        length += 1;
        ptr++;
    }

    int mid;
    int left = 0;
    int right = length - 1;

    while (left <= right)
    {
        mid = left + (right - left) / 2;

        if (strcmp(words_array[mid], word) == 0)
            return mid;

        if (strcmp(words_array[mid], word) > 0)
            right = mid - 1;
        else
            left = mid + 1;
    }
    return -1;
}

int linear_search(char *word, char **words_array)
{
    char **ptr = words_array;
    while (*ptr != NULL)
    {
        if (strcmp(*ptr, word) == 0)
        {
            return 0;
        }
        ptr++;
    }

    return -1;
}

char **create_word_array_from_file(char *file_name)
{
    char **words_array;
    char *buffer = NULL;
    size_t linecap = 0;
    ssize_t numchars;
    int i = 0;

    FILE *fp;
    fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "error opening file: %s", strerror(errno));
        exit(0);
    }

    //Count number of words in file for malloc
    int words = 0;
    char ch;
    while ((ch = fgetc(fp)) != EOF)
    {
        // Check words
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\0')
            words++;
    }
    rewind(fp);

    words_array = malloc(words * sizeof(char *));

    while ((numchars = getline(&buffer, &linecap, fp)) > 0)
    {
        words_array[i] = malloc(numchars * sizeof(char));
        if (buffer[numchars - 1] == '\n')
        {
            buffer[numchars - 1] = '\0';
        }
        strcpy(words_array[i], buffer);

        i++;
    }
    fclose(fp);

    return words_array;
}

struct WordQueue *create_words_queue_from_file(char *file_name)
{
    struct WordQueue *words_queue;
    struct WordNode *word;
    char *buffer = NULL;
    size_t linecap = 0;
    ssize_t numchars;
    int i = 0;

    FILE *fp;
    fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "error opening file: %s", strerror(errno));
        exit(0);
    }

    //Count number of words in file for initalizing queue
    int words = 0;
    char ch;
    while ((ch = fgetc(fp)) != EOF)
    {
        // Check words
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\0')
            words++;
    }
    rewind(fp);

    words_queue = allocate_word_queue_with_capacity(words);
    while ((numchars = getline(&buffer, &linecap, fp)) > 0)
    {
        word = create_new_word_node(buffer);
        enqueue_word(words_queue, word);
        i++;
    }
    fclose(fp);

    return words_queue;
}

/********************************* Wrapper functions **************************************/
void remove_newline_from_string(char *str)
{
    str[strlen(str) - 1] = '\0';
}

void posix_error(int code, char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(code));
    exit(EXIT_FAILURE);
}

void unix_error(char *msg) /* Unix-style error */
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(EXIT_FAILURE);
}

void Pthread_create(pthread_t *tid, pthread_attr_t *attr,
                    void *(*start_func)(void *), void *arg)
{
    int rc;
    if ((rc = pthread_create(tid, attr, start_func, arg)) != 0)
        posix_error(rc, "Pthread_create error");
}

void Pthread_detach(pthread_t tid)
{
    int rc;

    if ((rc = pthread_detach(tid)) != 0)
        posix_error(rc, "Pthread_detach error");
}
