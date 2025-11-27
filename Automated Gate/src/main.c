#include "stm32_unict_lib.h"
#include <string.h>

typedef enum {
    OFF,
    OPENING,
    WAITING,
    CLOSING,
} phase;

int current_phase = OFF;
char gate[5] = "----";
int gate_counter = 0;
int waiting_counter = 0;
int led_counter = 0;
int toggle_counter = 0;

char *display[] = {
    "----",
    "--- ",
    "--  ",
    "-   ",
    "    "
};

void setup(void) {
    ClockConfig();
    DISPLAY_init();
    GPIO_init(GPIOB);
    GPIO_init(GPIOC);

    GPIO_config_input(GPIOB, 10); // X (PB10) Apertura
    GPIO_config_input(GPIOB, 4); //  Y (PB4)  Chiusura
    GPIO_config_input(GPIOB, 5); //  Z (PB5)  Fotocellula

    GPIO_config_output(GPIOB, 0); //LED rosso (PB0)

    GPIO_config_EXTI(GPIOB, EXTI10);
    GPIO_config_EXTI(GPIOB, EXTI4);
    GPIO_config_EXTI(GPIOB, EXTI5);

    EXTI_enable(EXTI10, FALLING_EDGE);
    EXTI_enable(EXTI4, FALLING_EDGE);
    EXTI_enable(EXTI5, FALLING_EDGE);

    TIM_init(TIM2);
    TIM_config_timebase(TIM2, 42000, 200); // 0.1 s
    TIM_enable_irq(TIM2, IRQ_UPDATE);
    TIM_on(TIM2);

    TIM_init(TIM3);
    TIM_config_timebase(TIM3, 42000, 200); // 0.1 s
    TIM_enable_irq(TIM3, IRQ_UPDATE);
    TIM_on(TIM3);
}

void loop(void) {
    DISPLAY_puts(0, gate);
}

void EXTI15_10_IRQHandler(void) {
    //Pushbutton X
    if(EXTI_isset(EXTI10)) {
        if(current_phase == OFF) {
            current_phase = OPENING;
        }
        if(current_phase == CLOSING) {
            current_phase = OPENING;
        }
        EXTI_clear(EXTI10);
    }
}

void EXTI4_IRQHandler(void) {
    //Pushbutton Y
    if(EXTI_isset(EXTI4)) {
        if(current_phase == WAITING) {
            current_phase = CLOSING;
        }

        EXTI_clear(EXTI4);
    }
}

void EXTI9_5_IRQHandler(void) {
    //Pushbutton Z
    if(EXTI_isset(EXTI5)) {
        if(current_phase == WAITING) {
            gate_counter = 0;
        }
        if(current_phase == CLOSING) {
            current_phase = OPENING;
        }
        EXTI_clear(EXTI5);
    }
}

void TIM2_IRQHandler(void) {
    if(TIM_update_check(TIM2)) { // Ogni 0.1 s
        if(current_phase == OPENING) {
            if(gate_counter < 25) {
                strcpy(gate, display[0]);
            }
            else if(gate_counter < 50) {
                strcpy(gate, display[1]);
            }
            else if(gate_counter < 75) {
                strcpy(gate, display[2]);
            }
            else if(gate_counter < 100) {
                strcpy(gate, display[3]);
            }
            else {
                strcpy(gate, display[4]);
                current_phase = WAITING;
                gate_counter = 0;
            }

        }
        else if(current_phase == WAITING) {
            ++waiting_counter;
            if(waiting_counter > 40) {
                current_phase = CLOSING;
            }
        }
        else if(current_phase == CLOSING) {
            if(gate_counter < 25) {
                strcpy(gate, display[4]);
            }
            else if(gate_counter < 50) {
                strcpy(gate, display[3]);
            }
            else if(gate_counter < 75) {
                strcpy(gate, display[2]);
            }
            else if(gate_counter < 100) {
                strcpy(gate, display[1]);
            }
            else {
                strcpy(gate, display[0]);
                current_phase = OFF;
                gate_counter = 0;
            }
        }
        if(current_phase == OFF) {
            GPIO_write(GPIOB, 0, 0);
            strcpy(gate, "----");
        }
        gate_counter++;
        TIM_update_clear(TIM2);
    }
}

void TIM3_IRQHandler(void) {
    if(TIM_update_check(TIM3)) { // Ogni 0.1 s
        if(current_phase != OFF) {
            ++led_counter;     
            if(led_counter >= 5) {
                GPIO_toggle(GPIOB, 0);
                led_counter = 0;
            }
        }
        TIM_update_clear(TIM3);
    }
}

int main() {
    setup();
    for(;;) loop();
}