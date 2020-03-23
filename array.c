#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

struct WordsArray
{
    char **words;
    int length;
};

struct WordsArray create_word_array_from_file(char *file_name)
{
    struct WordsArray words_array;
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

    words_array.words = malloc(words * sizeof(char *));

    while ((numchars = getline(&buffer, &linecap, fp)) > 0)
    {
        words_array.words[i] = malloc(numchars * sizeof(char));
        strcpy(words_array.words[i], buffer);
        i++;
    }
    words_array.length = i;
    fclose(fp);

    return words_array;
}

int is_valid_word_in_words(char *word, struct WordsArray words_array)
{
    //Perform binary search using strcmp - return -1 if not found, else return the index
    int mid;
    int length = words_array.length;
    int left = 0;
    int right = length - 1;

    while (left <= right)
    {
        printf("%s", words_array.words[mid]);
        mid = left + (right - left) / 2;
        if (strcmp(words_array.words[mid], word) == 0)
            return mid;
        else if (strcmp(words_array.words[mid], word) > 0)
            right = mid - 1;
        else
            left = mid + 1;
    }
    return -1;
}

int main()
{
    struct WordsArray words_array = create_word_array_from_file(DEFAULT_DICTIONARY);

    printf("%d", is_valid_word_in_words("a", words_array));
    // printf("%d", strcmp("aa", "cdscdsbb"));
}

//TODO: Binary search not working properly