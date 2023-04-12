#ifndef READER_HEADER
#define READER_HEADER
#include <stdio.h>
#include <semaphore.h>

enum{
    UPPER_CASE,
    LOWER_CASE
};

FILE** files;

struct readerAttrs{
    int mode;
    sem_t sem;
};

char makeUpperCase(char c);
char makeLowerCase(char c);

char getNext(FILE *file, char(*changer)(char));

void* runReader(void* arg);

#endif
