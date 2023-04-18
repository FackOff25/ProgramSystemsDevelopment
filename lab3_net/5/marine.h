#ifndef MARINE_HEADER
#define MARINE_HEADER

#include "protocol.h"

typedef enum{
    WRONG,
    MISS,
    HIT,
    KILL
} MarineAnswer;

MarineAnswer getAnswerFromStr(char* str, CODINGS coding);

int shouldFireAgain(MarineAnswer answer);

#endif