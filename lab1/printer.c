#include <stdio.h>
#include "elevators.h"

void print_floors(void){
    printf("|");
    int i = 1;
    for (; i <= FLOORS; i++)
        printf((i == FLOORS) ? "%d|" : "%d_", i % 10);
    printf(" \n");
}

void print_buttons(unsigned int buttons, char id, const char *name){
    printf(" |");
    int i = 0;
    for (; i < FLOORS; i++)
        printf((buttons & (1 << i)) ? "*|" : " |");
    printf(" :%s \n", name);
}

void print_elevator(const struct ELEVATOR *pe, char id, const char *name){
    char *idle = "I-";
    char *up = "I-";
    char *down = "I-";
    char *stop = "I-";
    char *wait = "O-";
    char *str = idle;
    switch (pe->state)
    {
    case E_IDLE:
        str = idle;
        break;
    case E_MOVING_UP:
        str = up;
        break;
    case E_MOVING_DOWN:
        str = down;
        break;
    case E_STOP:
        str = stop;
        break;
    case E_WAIT:
        str = wait;
        break;
    }
    printf(" |");
    unsigned int i = 0;
    for (; i < FLOORS; i++)
        printf((i == pe->floor) ? "%s" : (i == (FLOORS - 1)) ? "-|"
                                                             : "--",
               str);
    printf(" :%s \n", name);
    print_buttons(pe->buttons, id, name);
}
