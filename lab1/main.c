#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/poll.h>

#include "elevators.h"
#include "printer.h"

#define READ_FD 0
#define WRITE_FD 1

struct termios savetty;
void set_noncanon(int set)
{
    struct termios tty;
    if (set)
    {
        if (!isatty(0))
        { /*Проверка: стандартный ввод - терминал?*/
            fprintf(stderr, "stdin not terminal\n");
            exit(1); /* Ст. ввод был перенаправлен на файл, канал и т.п. */
        };

        tcgetattr(0, &tty);
        savetty = tty; /* Сохранить упр. информацию канонического режима */
        tty.c_lflag &= ~(ICANON | ECHO | ISIG);
        tty.c_cc[VMIN] = 1;
        tcsetattr(0, TCSAFLUSH, &tty);
    }
    else
    {
        // restore the former settings
        tcsetattr(0, TCSAFLUSH, &savetty);
    }
}

// маску в номер этажа
unsigned int mask_to_floor(unsigned int u)
{
    unsigned int floor = 0;
    while (u > 1)
    {
        floor++;
        u >>= 1;
    }
    return floor;
}

int pipes[NUM_PIPES][2];
struct ELEVATOR pe;
struct ELEVATOR fe;
unsigned int cb = 0; // ненезначенные ни одному из лифтов вызовы
unsigned int pr = 0; // вызовы пассажирского лифта
unsigned int fr = 0; // вызовы грузового
unsigned int pb = 0; // пасс кнопки
unsigned int fb = 0; // груз кнопки

void print_all()
{
    printf("\033[0;0H"); // set pos
    print_floors();
    printf("\n");
    print_buttons(cb | pr | fr, "Buttons");
    printf("\n");
    print_elevator(&pe, "Passanger");
    printf("\n");
    print_elevator(&fe, "Freight");
    printf("\n");
}

void send_to_elevators(struct E_REQ prq, struct E_REQ frq)
{
    if (prq.cabin_press || prq.goto_floor)
    { // делаем запрос/команду лифту если нажаты кнопки кабины или есть вызов с этажа
        write(pipes[BU_WRITE_EL1][WRITE_FD], &prq, sizeof(prq));
    }
    frq.cabin_press = fb;
    if (frq.cabin_press || frq.goto_floor)
    { // делаем запрос/команду лифту если нажаты кнопки кабины или есть вызов с этажа
        write(pipes[BU_WRITE_EL2][WRITE_FD], &frq, sizeof(frq));
    }
}

void read_elevators(int sig)
{
    fd_set rfds;
    struct timeval tv = {0, 0};
    FD_ZERO(&rfds);
    FD_SET(pipes[BU_READ_EL1][READ_FD], &rfds);
    FD_SET(pipes[BU_READ_EL2][READ_FD], &rfds);
    if (select(MAX(pipes[BU_READ_EL1][READ_FD], pipes[BU_READ_EL2][READ_FD]) + 1, &rfds, NULL, NULL, &tv) > 0)
    {
        if (FD_ISSET(pipes[BU_READ_EL1][READ_FD], &rfds))
        {
            read(pipes[BU_READ_EL1][READ_FD], &pe,
                 sizeof(pe));
            cb &= ~pe.reqdone;
            pb = pe.buttons;
        }
        if (FD_ISSET(pipes[BU_READ_EL2][READ_FD], &rfds))
        {
            read(pipes[BU_READ_EL2][READ_FD], &fe,
                 sizeof(fe));
            cb &= ~fe.reqdone;
            fb = fe.buttons;
        }
    }
    pr = pe.request;
    fr = fe.request;
    print_all();
    struct E_REQ prq = {0, 0}; // команда(запрос) пассажирскому
    struct E_REQ frq = {0, 0}; // команда(запрос) грузовому
    if ((pe.state == E_IDLE || fe.state == E_IDLE) && cb)
    {
        unsigned int pfmask = (1 << pe.floor);
        unsigned int ffmask = (1 << fe.floor);
        unsigned cbbuf = cb;
        unsigned int h = highest_bit_mask(cbbuf);
        while ((h == pr || h == fr) && h != 0)
        {
            cbbuf &= ~h;
            h = highest_bit_mask(cbbuf);
        }
        if (h != 0)
        {
            if ((pe.state == E_IDLE) && (fe.state != E_IDLE))
            {
                cb |= pr; // сохраняем старый вызов
                pr = h;
                prq.goto_floor = h;
            }
            else if ((fe.state == E_IDLE) && (pe.state != E_IDLE))
            {
                cb |= fr; // сохраняем старый вызов
                fr = h;
                frq.goto_floor = h;
            }
            else if ((pe.state == E_IDLE) && (fe.state == E_IDLE))
            {
                if (abs(mask_to_floor(h) - pe.floor) <= abs(mask_to_floor(h) - fe.floor))
                { // пассажирский ближе
                    if (!pe.buttons)
                    {
                        cb |= pr; // сохраняем старый вызов
                        pr = h;
                        prq.goto_floor = h;
                    }
                    else
                    {
                        if (!fe.buttons)
                        {
                            cb |= fr; // сохраняем старый вызов
                            fr = h;
                            frq.goto_floor = h;
                        }
                    }
                }
                else
                {
                    if (!fe.buttons)
                    {
                        cb |= fr; // сохраняем старый вызов
                        fr = h;
                        frq.goto_floor = h;
                    }
                    else
                    {
                        if (!pe.buttons)
                        {

                            cb |= pr; // сохраняем старый вызов
                            pr = h;
                            prq.goto_floor = h;
                        }
                    }
                }
            }
        }
    }
    prq.cabin_press = pb;
    frq.cabin_press = fb;
    send_to_elevators(prq, frq);
}

int main()
{
    int child1, child2;
    // для взаимодействия между родительским и дочерними процессами
    // используется каналы(pipes)
    pipe(pipes[BU_READ_EL1]);
    pipe(pipes[BU_WRITE_EL1]);
    pipe(pipes[BU_READ_EL2]);
    pipe(pipes[BU_WRITE_EL2]);
    if ((child1 = fork()) == 0)
    {                                                      // child 1
        dup2(pipes[BU_READ_EL1][WRITE_FD], STDOUT_FILENO); // stdout теперь пишет в канал
        dup2(pipes[BU_WRITE_EL1][READ_FD], STDIN_FILENO);  // stdin теперь читает из канала
        // закрываем не используемые в этом процессе, но унаследованные от родительского концы каналов
        close(pipes[BU_READ_EL1][READ_FD]);
        close(pipes[BU_WRITE_EL1][WRITE_FD]);
        close(pipes[BU_READ_EL2][READ_FD]);
        close(pipes[BU_READ_EL2][WRITE_FD]);
        close(pipes[BU_WRITE_EL2][READ_FD]);
        close(pipes[BU_WRITE_EL2][WRITE_FD]);
        struct ELEVATOR pe;
        elevator_init(&pe, 2); // speed
        elevator_run(&pe);
        exit(0);
    }
    // закрываем концы каналов которые используются дочерним процессом
    close(pipes[BU_READ_EL1][WRITE_FD]);
    close(pipes[BU_WRITE_EL1][READ_FD]);
    if ((child2 = fork()) == 0)
    { // child 2
        dup2(pipes[BU_READ_EL2][WRITE_FD], STDOUT_FILENO);
        dup2(pipes[BU_WRITE_EL2][READ_FD], STDIN_FILENO);
        // закрываем не используемые в этом процессе, но унаследованные от родительского концы каналов
        close(pipes[BU_READ_EL2][READ_FD]);
        close(pipes[BU_WRITE_EL2][WRITE_FD]);
        close(pipes[BU_READ_EL1][READ_FD]);
        close(pipes[BU_READ_EL1][WRITE_FD]);
        close(pipes[BU_WRITE_EL1][READ_FD]);
        close(pipes[BU_WRITE_EL1][WRITE_FD]);
        struct ELEVATOR fe;
        elevator_init(&fe, 1); // speed
        elevator_run(&fe);
        exit(0);
    }
    signal(SIGREAD, read_elevators);
    // закрываем концы каналов которые используются дочерним процессом
    close(pipes[BU_READ_EL2][WRITE_FD]);
    close(pipes[BU_WRITE_EL2][READ_FD]);
    /////////////////////////////////////////////////////
    printf("\033[2J"); // clear
    int exit = 0;
    struct pollfd p = {0, 1, 0};
    set_noncanon(1);
    do
    {
        int c;
        read(STDIN_FILENO, &c, 1);
        if (c == 27)
        {                                // esc exit
            struct E_REQ prq = {0, 0};   // команда(запрос) пассажирскому
            struct E_REQ frq = {0, 0};   // команда(запрос) грузовому
            prq.goto_floor = EXIT_FLOOR; // спец значение флаг выхода из процесса
            frq.goto_floor = EXIT_FLOOR;
            send_to_elevators(prq, frq);
            set_noncanon(0);
            break;
        }
        else if (c == '0')
        {
            cb |= (1 << 9);
        }
        else if ((c >= '1') && (c <= '9'))
        {
            cb |= (1 << (c - '1'));
        }
        else
        {
            switch (c)
            {
            case 'q':
                pb |= (1 << 0);
                break;
            case 'w':
                pb |= (1 << 1);
                break;
            case 'e':
                pb |= (1 << 2);
                break;
            case 'r':
                pb |= (1 << 3);
                break;
            case 't':
                pb |= (1 << 4);
                break;
            case 'y':
                pb |= (1 << 5);
                break;
            case 'u':
                pb |= (1 << 6);
                break;
            case 'i':
                pb |= (1 << 7);
                break;
            case 'o':
                pb |= (1 << 8);
                break;
            case 'p':
                pb |= (1 << 9);
                break;
            case 'a':
                fb |= (1 << 0);
                break;
            case 's':
                fb |= (1 << 1);
                break;
            case 'd':
                fb |= (1 << 2);
                break;
            case 'f':
                fb |= (1 << 3);
                break;
            case 'g':
                fb |= (1 << 4);
                break;
            case 'h':
                fb |= (1 << 5);
                break;
            case 'j':
                fb |= (1 << 6);
                break;
            case 'k':
                fb |= (1 << 7);
                break;
            case 'l':
                fb |= (1 << 8);
                break;
            case ';':
                fb |= (1 << 9);
                break;
            }
        }
        struct E_REQ prq = {0, 0}; // команда(запрос) пассажирскому
        struct E_REQ frq = {0, 0}; // команда(запрос) грузовому
        unsigned int pfmask = (1 << pe.floor);
        unsigned int ffmask = (1 << fe.floor);
        unsigned int cbbuf = cb;
        unsigned int h = highest_bit_mask(cbbuf);
        while ((h == pr || h == fr) && h != 0)
        {
            cbbuf &= ~h;
            h = highest_bit_mask(cbbuf);
        }
        if (h != 0)
        {
            if ((pe.state == E_IDLE) && (fe.state != E_IDLE))
            {
                cb |= pr; // сохраняем старый вызов
                pr = h;
                prq.goto_floor = h;
            }
            else if ((fe.state == E_IDLE) && (pe.state != E_IDLE))
            {
                cb |= fr; // сохраняем старый вызов
                fr = h;
                frq.goto_floor = h;
            }
            else if ((pe.state == E_IDLE) && (fe.state == E_IDLE))
            {
                if (abs(mask_to_floor(h) - pe.floor) <= abs(mask_to_floor(h) - fe.floor))
                { // пассажирский ближе
                    if (!pe.buttons)
                    {
                        cb |= pr; // сохраняем старый вызов
                        pr = h;
                        prq.goto_floor = h;
                    }
                    else
                    {
                        if (!fe.buttons)
                        {
                            cb |= fr; // сохраняем старый вызов
                            fr = h;
                            frq.goto_floor = h;
                        }
                    }
                }
                else
                {
                    if (!fe.buttons)
                    {
                        cb |= fr; // сохраняем старый вызов
                        fr = h;
                        frq.goto_floor = h;
                    }
                    else
                    {
                        if (!pe.buttons)
                        {

                            cb |= pr; // сохраняем старый вызов
                            pr = h;
                            prq.goto_floor = h;
                        }
                    }
                }
            }
        }
        prq.cabin_press = pb;
        frq.cabin_press = fb;
        send_to_elevators(prq, frq);
    } while (!exit);

    set_noncanon(0);
    // ждем окончания дочерних процесссов
    waitpid(child1, 0, 0);
    waitpid(child2, 0, 0);
    return 0;
}
