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
        if (buffer[numchars - 1] == '\n')
        {
            buffer[numchars - 1] = '\0';
        }
        strcpy(words_array.words[i], buffer);

        i++;
    }
    words_array.length = i;
    fclose(fp);

    return words_array;
}

// TODO: Buggy seach
int binary_search(char *word, struct WordsArray words_array)
{
    //Perform binary search using strcmp - return -1 if not found, else return the index
    int length = words_array.length;
    int mid;
    int left = 0;
    int right = length - 1;

    while (left <= right)
    {
        mid = left + (right - left) / 2;

        if (strcmp(words_array.words[mid], word) == 0)
            return mid;

        if (strcmp(words_array.words[mid], word) > 0)
            right = mid - 1;
        else
            left = mid + 1;
    }
    return -1;
}

// to double check when binary search fails
int linear_search(char *word, struct WordsArray words_array)
{
    for (int i; i < words_array.length; i++)
    {
        if (strcmp(words_array.words[i], word) == 0)
        {
            return i;
        }
    }
    return -1;
}

int main()
{
    struct WordsArray words_array = create_word_array_from_file(DEFAULT_DICTIONARY);
    int word_found_index;
    if ((word_found_index = binary_search("zoid", words_array)) == -1) //if binary search unsuccessful, handle edgecase since bsearch is buggy
    {

        printf("%dHere\n", word_found_index);
        word_found_index = linear_search("zoid", words_array);
        printf("%dHere\n", word_found_index);
    }
    printf("%d\n", word_found_index);
    return 0;
}
