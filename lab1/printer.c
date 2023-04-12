#include <stdio.h>
#include <string.h>
#include "elevators.h"

void print_floors(void){
    printf("|");
    int i = 1;
    for (; i <= FLOORS; i++)
        printf((i != FLOORS) ? "%d " : "%d|", i % 10);
    printf(" \n");
    fflush (stdout);
}

void print_buttons(unsigned int buttons, const char *name){
    printf("|");
    int i = 0;
    for (; i < FLOORS; i++)
        printf((buttons & (1 << i)) ? "*|" : " |");
    printf(" %s \n", name);
    fflush (stdout);
}

void print_elevator(const struct ELEVATOR *pe, const char *name){
    char *closed = "I-";
    char *opened = "O-";
    char *str = pe->passangers > 9 ? "" : "-";
    printf("|");
    unsigned int i = 0;
    for (; i < FLOORS; i++)
        printf((i == pe->floor) ? "%d%s" : (i == (FLOORS - 1)) ? "-|": "--",pe->passangers, str);
    printf(" %s\n", name);
    print_buttons(pe->buttons, "");
    fflush (stdout);
}
