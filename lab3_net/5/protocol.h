#ifndef PROTOCOL_HEADER
#define PROTOCOL_HEADER

typedef enum{
    NO_CODING,
    LATIN,
    CYRILLIC
} CODINGS;

typedef struct{
    CODINGS coding;
    char* message;
} Message;

#define CODING_ERROR "ERR CODING"

int parseMessage(char *message, int len, Message *parsed);
int makeMessage(Message *message, int len, char *made);
int makeCodingError(char *made);
#endif