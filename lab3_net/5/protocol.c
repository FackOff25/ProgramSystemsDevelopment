#include "protocol.h"

#include <string.h>
#include <stdio.h>

int parseMessage(char *message, int len, Message *parsed)
{
    parsed->coding = message[0];
    memcpy(parsed->message, message + 1, len - 1);
    return 0;
};

int makeMessage(Message *message, int len, char *made)
{
    made[0] = message->coding;
    memcpy(made + 1, message->message, len - 1);
    return 0;
};

int makeErrorMessage(char *made)
{
    memcpy(made, ERROR_MESSAGE, sizeof(char) * 11);
};
