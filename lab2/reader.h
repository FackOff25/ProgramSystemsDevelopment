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

int isUpperCase(char c);
int isLowerCase(char c);

char getNext(FILE *file, int(*verifier)(char));

void* runReader(void* arg);

#endif
