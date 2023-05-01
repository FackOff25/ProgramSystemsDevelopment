#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/poll.h>
#include "elevators.h"

// маска старшего установленного бита
unsigned int highest_bit_mask(unsigned int u)
{
    unsigned int r = 0;
    while (u)
    {
        u >>= 1;
        r <<= 1;
        r |= 1;
    }
    return ((++r) >> 1);
}

// маска младшего установленного бита
unsigned int lowest_bit_mask(unsigned int u)
{
    if (u == 0)
        return 0;
    unsigned int r = 1;
    while ((u & r) == 0)
    {
        r <<= 1;
    }
    return r;
}

void elevator_init(struct ELEVATOR *pe, int speed)
{
    pe->floor = 0;
    pe->buttons = 0;
    pe->request = 0;
    pe->state = E_IDLE;
    pe->speed = speed;
    pe->reqdone = 0;
    pe->passangers = 0;
}

// изменилось ли состояние лифта
int elevator_state_eq(struct ELEVATOR *a, struct ELEVATOR *b)
{
    return (
        (a->buttons == b->buttons) &&
        (a->floor == b->floor) &&
        (a->request == b->request) &&
        (a->state == b->state) &&
        (a->reqdone == b->reqdone));
}

void update_state(struct ELEVATOR *pe)
{
    write(STDOUT_FILENO, pe, sizeof(*pe));
    kill(getppid(), SIGREAD);
}

// функция процесса обработчика лифта
void elevator_run(struct ELEVATOR *pe)
{
    struct E_REQ req;
    struct ELEVATOR old;
    unsigned int tick = 1000000 / pe->speed;
    int gotreq = 1; // посылаем статус при первом запуске
    struct pollfd p = {0, 1, 0};
    int exit = 0;
    while (exit == 0)
    {
        //////////////////////////////////////////////////////////////////////////////////////////
        unsigned int fmask = (1 << pe->floor); // маска текущего этажа
        pe->reqdone = 0;
        old = *pe; // сохраняем текущее состояние
        //////////////////////////////////////////////////////////////////////////////////////////
        memset(&req, 0, sizeof(req)); // заполняем нулями весь req
        if (pe->state == E_IDLE && pe->buttons == 0 && pe->request == 0)
        {
            update_state(pe);
            read(STDIN_FILENO, &req, sizeof(req));
            // обрабатываем запрос
            gotreq = 1;
            if (req.goto_floor == EXIT_FLOOR)
            {
                exit = 1;
                break;
            }
            if (req.goto_floor)
            {
                pe->request = req.goto_floor;
            }
            if (!pe->request)
            { // игнорирование нажатия кнопок кабины если уже отрабатывает request
                if (req.cabin_press && pe->passangers != 0)
                { // got reqs inside cabin
                    pe->buttons |= req.cabin_press;
                }
            }
        }
        while (poll(&p, POLLIN, 0))
        { // ожидает готовые файловые дескрипотры
            read(STDIN_FILENO, &req, sizeof(req));
            // обрабатываем запрос
            gotreq = 1;
            if (req.goto_floor == EXIT_FLOOR)
            {
                exit = 1;
                break;
            }
            if (req.goto_floor)
            {
                pe->request = req.goto_floor;
            }
            if (!pe->request)
            { // игнорирование нажатия кнопок кабины если уже отрабатывает request
                if (req.cabin_press)
                { // got reqs inside cabin
                    pe->buttons |= req.cabin_press;
                }
            }
        }
        ///////////////////////////////////////////////////////////////////////////////////////////
        switch (pe->state)
        {
        case E_IDLE:
            if (pe->buttons)
            { // нажата кнопка внутри лифта, она приоритетней
                if (fmask & pe->buttons)
                { // этаж совпал
                    pe->state = E_STOP;
                }
                else if (pe->buttons < fmask)
                { // выше
                    pe->state = E_MOVING_DOWN;
                }
                else
                { // ниже
                    pe->state = E_MOVING_UP;
                }
            }
            else if (pe->request)
            {
                if (fmask & pe->request)
                { // этаж совпал
                  // assert(0 && "нельзя вызывать лифт на тот этаж на котором он находится"); pe->state = E_STOP;
                }
                else if (pe->request < fmask)
                {
                    pe->state = E_MOVING_DOWN;
                }
                else
                {
                    pe->state = E_MOVING_UP;
                }
            }
            break;
        case E_MOVING_UP:
            if (pe->buttons)
            { // внутри кто-то есть
                if (fmask & pe->buttons)
                {
                    pe->state = E_STOP;
                    pe->passangers--;
                }
            }
            else if (pe->request)
            {
                if (fmask & pe->request)
                {
                    pe->state = E_STOP;
                    pe->passangers++;
                }
                else if (fmask > pe->request)
                {
                    pe->state = E_MOVING_DOWN;
                }
            }
            else
            {
                pe->state = E_STOP;
            }
            break;
        case E_MOVING_DOWN:
            if (pe->buttons)
            { // внутри кто-то есть
                if (fmask & pe->buttons)
                {
                    pe->state = E_STOP;
                    pe->passangers--;
                }
            }
            if (pe->request)
            {
                if (fmask & pe->request)
                {
                    pe->state = E_STOP;
                    pe->passangers++;
                }
                else if (fmask < pe->request)
                {
                    pe->state = E_MOVING_UP;
                }
            }

            if ((pe->buttons | pe->request) == 0)
            {
                pe->state = E_STOP;
            }
            break;
        case E_STOP:
            if (pe->buttons)
            { // внутри кто-то есть
                if (fmask & pe->buttons)
                {
                    pe->buttons &= ~fmask;
                }
            }

            if (pe->request)
            {
                if (fmask & pe->request)
                {
                    pe->request &= ~fmask; // = 0
                }
            }

            pe->reqdone |= fmask; // fmask
            pe->state = E_WAIT;
            break;
        case E_WAIT:
            pe->state = E_IDLE;
            break;
        }
        if (pe->state == E_MOVING_UP)
        {
            pe->floor++;
            if (pe->floor > FLOORS)
            {
                pe->floor = FLOORS - 1;
                pe->state = E_STOP;
            };
        }
        else if (pe->state == E_MOVING_DOWN)
        {
            pe->floor--;
            if (pe->floor < 0)
            {
                pe->floor = 0;
                pe->state = E_STOP;
            };
        }
        if(pe->passangers < 0){
            pe->passangers = 0;
        }
        if (pe->passangers == 0){
            pe->buttons = 0;
        }
        // если состояние изменилось, сообщаем новое родителю
        update_state(pe);
        //////////////////////////////////////////////////////////////////////////////////////////
        usleep(tick);
    }
}
