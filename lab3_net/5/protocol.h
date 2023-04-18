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

#define ERROR_MESSAGE "ERROR"

int parseMessage(char *message, int len, Message *parsed);
int makeMessage(Message *message, int len, char *made);
int makeErrorMessage(char *made);
#endif