#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

#include "reader.h"

#define BUF_SIZE 256

int verify_args(int argc, char **argv)
{
    if (argc < 3)
        return 0;
}

FILE* openFile(char* fileName){
    FILE* file = fopen(fileName, "r");
    if (file == NULL){
        printf("%s can't be opened to read\n", fileName);
    }
    return file;
}

void nullifySem(sem_t *sem){
    while(sem_trywait(sem) != -1)
    ;
}

void makeSemOne(sem_t *sem){
    nullifySem(sem);
    sem_post(sem);
}

int readLine(char* str){
    fgets(str, BUF_SIZE, stdin);
    str[strcspn(str, "\n")] = '\0';
    return 1;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Need 2 file names as arguments\n");
        return 0;
    }

    files = malloc(sizeof(FILE*)*2);

    files[0] = openFile(argv[1]);
    files[1] = openFile(argv[2]);

    int mode = UPPER_CASE;
    struct readerAttrs attrs[2];
    attrs[0].mode = UPPER_CASE;
    attrs[1].mode = LOWER_CASE;

    sem_init(&(attrs[0].sem), 0, 0);
    sem_init(&(attrs[1].sem), 0, 0);
    
    pthread_t thread0, thread1;
    pthread_attr_t pattr;

    pthread_attr_init (&pattr);
    pthread_attr_setscope (&pattr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate (&pattr,PTHREAD_CREATE_JOINABLE);

    pthread_create(&thread0, &pattr, runReader, &(attrs[0]));
    pthread_create(&thread1, &pattr, runReader, &(attrs[1]));
    char filename[BUF_SIZE];
    while(readLine(filename)){
        if(strcmp(filename, "quit") == 0) 
            break;
        FILE* newReader = openFile(filename);
	if (newReader == NULL)
		continue;
        FILE* buf = files[1];
        files[1] = files[0];
        rewind(files[1]);
        files[0] = newReader;
        fclose(buf);

        makeSemOne(&(attrs[0].sem));
        makeSemOne(&(attrs[1].sem));
    }

    pthread_cancel(thread0);
    pthread_cancel(thread1);
    sem_destroy(&(attrs[0].sem));
    sem_destroy(&(attrs[1].sem));

    fclose(files[0]);
    fclose(files[1]);
    free(files);
    return 0;
}
