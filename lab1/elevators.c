#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/select.h>

#include "elevators.h"

void elevator_init(struct ELEVATOR *pe, int speed){
    pe->floor = 0;
    pe->buttons = 0;
    pe->request = 0;
    pe->state = E_IDLE;
    pe->speed = speed;
    pe->reqdone = 0;
}

// изменилось ли состояние лифта
int elevator_state_eq(struct ELEVATOR *a, struct ELEVATOR *b){
    return (
        (a->buttons == b->buttons) &&
        (a->floor == b->floor) &&
        (a->request == b->request) &&
        (a->state == b->state) &&
        (a->reqdone == b->reqdone));
}

// функция процесса обработчика лифта
void elevator_run(struct ELEVATOR *pe){
    struct E_REQ req;
    struct ELEVATOR old;
    unsigned int tick = 1000000/pe->speed;
    // 1, состояние ожидания сделано для возможности нажать на кнопку
    // в кабине когда лифт остановился приехав по вызову с этажа, т.е. в течении этого времени
    // лифт будет стоять на этаже и ожидать нажатия кногпок в кабине
    // если нажатий за это время не будет, лифт пойдет к след вызову с этажа
    int gotreq = 1; // посылаем статус при первом запуске
    while (1){
        //////////////////////////////////////////////////////////////////////////////////////////
        unsigned int fmask = (1 << pe->floor); // маска текущего этажа
        pe->reqdone = 0;
        old = *pe; // сохраняем текущее состояние
        //////////////////////////////////////////////////////////////////////////////////////////
        memset(&req, 0, sizeof(req)); // заполняем нулями весь req
        fd_set rfds;
        struct timeval tv = {0,
                             0};
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);                              // stdin_fileno = 1
        if (select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv) > 0){ // ожидает готовые файловые дескрипотры
            if (FD_ISSET(STDIN_FILENO, &rfds)) // если stdin член дескрипотра rfds
            {
                read(STDIN_FILENO, &req, sizeof(req));
                // обрабатываем запрос
                gotreq = 1;
                if (req.goto_floor == EXIT_FLOOR){
                    break;
                }
                if (req.goto_floor){
                    pe->request = req.goto_floor;
                }
                if (!pe->request){ // игнорирование нажатия кнопок кабины если уже отрабатывает request ?
                    if (req.cabin_press){ // got reqs inside cabin
                        if (!pe->buttons){ // не было нажато никаких кнопок
                            if ((pe->state == E_IDLE) || (pe->state == E_WAIT) /*|| (pe->state == E_STOP)*/){
                                pe->buttons = req.cabin_press;
                            }else{
                                // игнорируем(отменяем) нажатые кнопки если лифт уже едет по вызову с этажа
                            }
                        }else{
                            unsigned int lowermask = fmask - 1;
                            if (pe->buttons > fmask){// можно выше, уже нажатые кнопки выше текущего этажа
                                pe->buttons |= req.cabin_press & ~lowermask;
                            }else{ // можно ниже (равно?)
                                pe->buttons |= req.cabin_press & lowermask;
                            }
                        }
                    }
                }
            }
        }
        ///////////////////////////////////////////////////////////////////////////////////////////
        switch (pe->state){
        case E_IDLE:
            if (pe->buttons){ // нажата кнопка внутри лифта, она приоритетней
                if (fmask & pe->buttons){ // этаж совпал
                    pe->state = E_STOP;
                }else if (pe->buttons > fmask){ // выше
                    pe->state = E_MOVING_UP;
                }else{ // ниже
                    pe->state = E_MOVING_DOWN;
                }
            }else if (pe->request){
                if (fmask & pe->request){ // этаж совпал
                    // assert(0 && "нельзя вызывать лифт на тот этаж на котором он находится"); pe->state = E_STOP;
                }else if (pe->request > fmask){
                    pe->state = E_MOVING_UP;
                }else{
                    pe->state = E_MOVING_DOWN;
                }
            }
            break;
        case E_MOVING_UP:
            if (pe->buttons){ // внутри кто-то есть
                if (fmask & pe->buttons){
                    pe->state = E_STOP;
                }
            }else if (pe->request){
                if (fmask & pe->request)
                {
                    pe->state = E_STOP;
                }
            }else{
                assert(0 && "Едем без вызова ???");
            }
            break;
        case E_MOVING_DOWN:
            if (pe->buttons){ // внутри кто-то есть
                if (fmask & pe->buttons){
                    pe->state = E_STOP;
                }
            }

            if (pe->request){
                if (fmask & pe->request){
                    pe->state = E_STOP;
                }
            }

            if ((pe->buttons | pe->request) == 0){
                assert(0 && "Едем без вызова ???");
            }
            break;
        case E_STOP:
            if (pe->buttons){ // внутри кто-то есть
                if (fmask & pe->buttons)
                {
                    pe->buttons &= ~fmask;
                }
            }

            if (pe->request){
                if (fmask & pe->request)
                {
                    pe->request &= ~fmask; // = 0
                }
            }

            pe->reqdone |= fmask; // fmask
            usleep(tick);
            pe->state = E_WAIT;
            break;
        case E_WAIT:
                pe->state = E_IDLE;
            break;
        }
        if (pe->state == E_MOVING_UP){
                pe->floor++;
                assert(pe->floor < FLOORS);
        }else if (pe->state == E_MOVING_DOWN){
                assert(pe->floor > 0);
                pe->floor--;
        }
        //////////////////////////////////////////////////////////////////////////////////////////
        // если состояние изменилось, сообщаем новое родителю
        if (gotreq || !elevator_state_eq(&old, pe)){
            write(STDOUT_FILENO, pe,
                  sizeof(*pe));
            gotreq = 0;
        }
        //////////////////////////////////////////////////////////////////////////////////////////
        usleep(tick);
    }
}
