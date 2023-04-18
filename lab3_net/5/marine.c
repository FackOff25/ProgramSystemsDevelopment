#include "marine.h"

#include <string.h>

int shouldFireAgain(MarineAnswer answer)
{
    if (answer == MISS || answer == WRONG)
    {
        return 0;
    }
    return 1;
};

MarineAnswer getAnswerFromStrLatin(char *str)
{
    if (strcmp(str, "miss") == 0)
    {
        return MISS;
    }
    else if (strcmp(str, "hit") == 0)
    {
        return HIT;
    }
    else if (strcmp(str, "kill") == 0)
    {
        return KILL;
    }
    else
    {
        return WRONG;
    }
};

MarineAnswer getAnswerFromStrCyrillic(char *str)
{
    if (!strcmp(str, "мимо"))
    {
        return MISS;
    }
    else if (!strcmp(str, "ранил"))
    {
        return HIT;
    }
    else if (!strcmp(str, "убил"))
    {
        return KILL;
    }
    else
    {
        return WRONG;
    }
};

MarineAnswer getAnswerFromStr(char *str, CODINGS coding)
{
    if (coding == LATIN)
    {
        return getAnswerFromStrLatin(str);
    }
    else if (coding == CYRILLIC)
    {
        return getAnswerFromStrCyrillic(str);
    }
    else
    {
        return WRONG;
    }
};
