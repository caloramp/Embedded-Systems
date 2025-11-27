#include "stm32_unict_lib.h"
#include "prenotation_queue.h"
#include <stdio.h>

#define MAX_PRENOTATIONS 10

typedef enum {
    NONE,
    FIRST_FLOOR,
    SECOND_FLOOR,
    THIRD_FLOOR
} floor_t;

typedef enum {
    WORKING,
    STOPPED,
    CONFIGURATION
} status_t;

typedef enum {
    OFF,
    CLOSING,
    RUNNING,
    OPENING
} phase_t;

floor_t current_floor = FIRST_FLOOR;
floor_t next_stop = NONE;
Queue prenotations_queue;

status_t current_status = STOPPED;

int counter = 0;
int closing_duration = 15;
int running_duration = 10;
int opening_duration = 15;


void setup(void) {
    ClockConfig();
    DISPLAY_init();
    CONSOLE_init();

    GPIO_init(GPIOB);
    GPIO_init(GPIOC);

    GPIO_config_input(GPIOB, 10);
    GPIO_config_input(GPIOB, 4);
    GPIO_config_input(GPIOB, 5);
    GPIO_config_input(GPIOB, 6);

    GPIO_config_EXTI(GPIOB, EXTI10);
    GPIO_config_EXTI(GPIOB, EXTI4);
    GPIO_config_EXTI(GPIOB, EXTI5);
    GPIO_config_EXTI(GPIOB, EXTI6);

    EXTI_enable(EXTI10, FALLING_EDGE);
    EXTI_enable(EXTI4, FALLING_EDGE);
    EXTI_enable(EXTI5, FALLING_EDGE);
    EXTI_enable(EXTI6, FALLING_EDGE);

    TIM_init(TIM2);
    TIM_config_timebase(TIM2, 42000, 200); // 0.1 s
    TIM_enable_irq(TIM2, IRQ_UPDATE);
    TIM_on(TIM2);

    init_queue(&prenotations_queue);
    enqueue(&prenotations_queue, FIRST_FLOOR);
    enqueue(&prenotations_queue, SECOND_FLOOR);
    enqueue(&prenotations_queue, THIRD_FLOOR);
    enqueue(&prenotations_queue, FIRST_FLOOR);
    enqueue(&prenotations_queue, FIRST_FLOOR);
    print_queue(&prenotations_queue);

    dequeue(&prenotations_queue, &next_stop);
    printf("Estratto: %d\n", next_stop);
    print_queue(&prenotations_queue);
}

void loop(void) {
    char c;
    if(kbhit()) {
        c = readchar();
        if(c == 'a') {
                init_queue(&prenotations_queue);
    enqueue(&prenotations_queue, FIRST_FLOOR);
    enqueue(&prenotations_queue, SECOND_FLOOR);
    enqueue(&prenotations_queue, THIRD_FLOOR);
    enqueue(&prenotations_queue, FIRST_FLOOR);
    enqueue(&prenotations_queue, FIRST_FLOOR);
    print_queue(&prenotations_queue);

    dequeue(&prenotations_queue, &next_stop);
    printf("Estratto: %d\n", next_stop);
    print_queue(&prenotations_queue);
        }
    }
}

int main() {
    setup();
    for(;;) loop();
}
/*

void EXTI15_10_IRQHandler(void) {
    //Puxhbutton X
    if(EXTI_isset(EXTI10)) {
        if(current_status == STOPPED) {
            if(current_floor != FIRST_FLOOR) {
                next_stop = FIRST_FLOOR;
            }
        }
        else if(current_status == WORKING) {
            for(int i = 0; i < MAX_PRENOTATIONS; i++) {
                if(prenotations_buffer[i] == NONE) {
                    prenotations_buffer[i] = FIRST_FLOOR;
                    break;
                }
            }
        }
        EXTI_clear(EXTI10);
    }
}

void EXTI4_IRQHandler(void) {
    //Pushbutton Y
    if(EXTI_isset(EXTI4)) {
        if(current_status == STOPPED) {
            if(current_floor != SECOND_FLOOR) {
                next_stop = SECOND_FLOOR;
            }
        }
        else if(current_status == WORKING) {
            for(int i = 0; i < MAX_PRENOTATIONS; i++) {
                if(prenotations_buffer[i] == NONE) {
                    prenotations_buffer[i] = SECOND_FLOOR;
                    break;
                }
            }
        }
        EXTI_clear(EXTI4);
    }
}

void EXTI9_5_IRQHandler(void) {
    //Pushbutton Z
    if(EXTI_isset(EXTI5)) {
        if(current_status == STOPPED) {
            if(current_floor != THIRD_FLOOR) {
                next_stop = THIRD_FLOOR;
            }
        }
        else if(current_status == WORKING) {
            for(int i = 0; i < MAX_PRENOTATIONS; i++) {
                if(prenotations_buffer[i] == NONE) {
                    prenotations_buffer[i] = THIRD_FLOOR;
                    break;
                }
            }
        }
        EXTI_clear(EXTI5);
    }
}


void TIM2_IRQHandler(void) {
    if(TIM_update_check(TIM2)) {
        TIM_update_clear(TIM2);
    }
}
    */