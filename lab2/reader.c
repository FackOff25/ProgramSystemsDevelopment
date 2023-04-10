#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>

#include "reader.h"

int isUpperCase(char c)
{
    return (c >= 'A') && (c <= 'Z');
};

int isLowerCase(char c)
{
    return (c >= 'a') && (c <= 'z');
};

char getNext(FILE *file, int (*verifier)(char))
{
    char c = 0;
    while (!verifier(c) && c != EOF)
    {
        c = fgetc(file);
    }
    return c;
};

void *runReader(void *arg)
{
    struct readerAttrs *args = (struct readerAttrs *)arg;
    int (*verifyer)(char);
    FILE *file;
    if (args->mode == UPPER_CASE)
    {
        verifyer = isUpperCase;
    }
    else if (args->mode == LOWER_CASE)
    {
        verifyer = isLowerCase;
    }
    else
    {
        printf("Wrong mode parameter");
        void* returnValue;
        return returnValue;
    }

    do
    {
        char c = getNext(files[args->mode], verifyer);
        if (c != EOF)
        {
            do
            {
                printf("%c\n", c);
                c = getNext(files[args->mode], verifyer);
                sleep(1);
            } while (c != EOF);
        }

        sem_wait(&(args->sem));
    } while (1);
};
