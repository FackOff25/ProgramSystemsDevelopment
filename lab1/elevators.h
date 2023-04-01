#ifndef ELEVATOR_HEADER
#define ELEVATOR_HEADER
typedef enum{
    E_IDLE,
    E_MOVING_UP,
    E_MOVING_DOWN,
    E_STOP,
    E_WAIT,
} ELEVATOR_STATE;

struct ELEVATOR{
    unsigned int floor;
    unsigned int buttons;
    unsigned int request;
    ELEVATOR_STATE state;
    unsigned int speed;
    unsigned int reqdone;
};

enum{
    BU_READ_EL1,
    BU_WRITE_EL1,
    BU_READ_EL2,
    BU_WRITE_EL2,
    NUM_PIPES
};

struct E_REQ{
    unsigned int cabin_press;
    unsigned int goto_floor;
};

#define FLOORS 10
#define EXIT_FLOOR 0xFFFFFFFF
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

//printer
void print_floors(void);
void print_buttons(unsigned int buttons, char id, const char *name);
void print_elevator(const struct ELEVATOR *pe, char id, const char *name);

//elevator
void elevator_init(struct ELEVATOR *pe, int speed);
int elevator_state_eq(struct ELEVATOR *a, struct ELEVATOR *b);
void elevator_run(struct ELEVATOR *pe);
#endif
