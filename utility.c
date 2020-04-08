#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"

//FIFO Queue for client servicing and logging
struct Queue
{
    int front, rear, size;
    unsigned capacity;
    int *array;
};

struct Queue *allocate_queue_with_capacity(unsigned capacity)
{
    struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->array = (int *)malloc(queue->capacity * sizeof(int));
    return queue;
}

int queue_is_full(struct Queue *queue)
{
    return (queue->size == queue->capacity);
}

int queue_is_empty(struct Queue *queue)
{
    return (queue->size == 0);
}

int enqueue(struct Queue *queue, int item)
{
    if (queue_is_full(queue)) //Error
        return -1;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;

    return 0;
}

int dequeue(struct Queue *queue)
{
    if (queue_is_empty(queue)) //Error
        return -1;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;

    return item;
}

//Spell checking helpers
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

int binary_search(char *word, char **words_array)
{
    //Perform binary search using strcmp - return -1 if not found, else return the index
    int length = 0;
    char **ptr = words_array;
    while (*ptr != NULL)
    {
        length += 1;
        *ptr++;
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
        *ptr++;
    }

    return -1;
}

// Wrapper functions
void remove_newline_from_string(char *str)
{
    str[strlen(str) - 1] = '\0';
}

void posix_error(int code, char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(code));
    exit(0);
}

void unix_error(char *msg) /* Unix-style error */
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
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
