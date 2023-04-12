#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>

#include "reader.h"

char makeUpperCase(char c)
{
	if((c >= 'a') && (c <= 'z'))
		c -= 32;
    	return c;
};

char makeLowerCase(char c)
{
    if((c >= 'A') && (c <= 'Z'))
	c += 32;
    return c;
};

char getNext(FILE *file, char (*changer)(char))
{
    char c = 0;
    c = fgetc(file);
    return changer(c);
};

void *runReader(void *arg)
{
    struct readerAttrs *args = (struct readerAttrs *)arg;
    char (*changer)(char);
    FILE *file;
    if (args->mode == UPPER_CASE)
    {
        changer = makeUpperCase;
    }
    else if (args->mode == LOWER_CASE)
    {
        changer = makeLowerCase;
    }
    else
    {
        printf("Wrong mode parameter");
        void* returnValue;
        return returnValue;
    }

    do
    {
        char c = getNext(files[args->mode], changer);
        if (c != EOF)
        {
            do
            {
                printf("%c\n", c);
                c = getNext(files[args->mode], changer);
                sleep(1);
            } while (c != EOF);
        }

        sem_wait(&(args->sem));
    } while (1);
};
