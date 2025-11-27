#include "stm32_unict_lib.h"
#include <stdio.h>

typedef enum {
    SETUP,
    READY,
    WORKING,
    PAUSED,
    FINISHED
} state_t;

int actual_SS = 0;
int actual_CC = 0;
int setted_SS = 0;
int setted_CC = 0;
char to_display[5];
int yellow_flash_counter = 0;

state_t current_status = SETUP;

void setup() {
    ClockConfig(); //Imposta la velocità di clock a 84 MHz
    DISPLAY_init(); //Inizializza il display
    GPIO_init(GPIOB); //Inizializza la linea GPIO B
    GPIO_init(GPIOC); //Inizializza la linea GPIO C

    GPIO_config_input(GPIOB, 10); //Configura il pulsante X (PB10) come input
    GPIO_config_input(GPIOB, 4); //Configura il pulsante Y (PB4) come input
    GPIO_config_input(GPIOB, 5); //Configura il pulsante Z (PB5) come input
    GPIO_config_input(GPIOB, 5); //Configura il pulsante T (PB6) come input

    GPIO_config_EXTI(GPIOB, EXTI10); //Collega il pulsante X (PB10) all'EXTI10
    GPIO_config_EXTI(GPIOB, EXTI4); //Collega il pulsante Y (PB4) all'EXTI4
    GPIO_config_EXTI(GPIOB, EXTI5); //Collega il pulsante Z (PB5) all'EXTI5
    GPIO_config_EXTI(GPIOB, EXTI6); //Collega il pulsante T (PB6) all'EXTI6

    EXTI_enable(EXTI10, FALLING_EDGE); //Attiva la linea EXTI10 quando si verifica un falling edge  
    EXTI_enable(EXTI4, FALLING_EDGE); //Attiva la linea EXTI4 quando si verifica un falling edge  
    EXTI_enable(EXTI5, FALLING_EDGE); //Attiva la linea EXTI5 quando si verifica un falling edge  
    EXTI_enable(EXTI6, FALLING_EDGE); //Attiva la linea EXTI6 quando si verifica un falling edge  

    GPIO_config_output(GPIOB, 8); //Configura i : del display (PB8) come output
    GPIO_config_output(GPIOC, 2); //Configura il LED Giallo (PC2) come output
    GPIO_config_output(GPIOC, 3); //Configura il LED Verde (PC3) come output

    TIM_init(TIM2); //Inizializza il timer TIM2
    TIM_config_timebase(TIM2, 42000, 20); //0.01 s
    TIM_enable_irq(TIM2, IRQ_UPDATE); //Abilita le IRQ su TIM2
    TIM_on(TIM2);

    sprintf(to_display, "%02d%02d", setted_SS, setted_CC);
    GPIO_write(GPIOC, 3, 1);

}

void loop() {
    DISPLAY_puts(0, to_display);
    GPIO_write(GPIOB, 8, 1);
}

int main() {
    setup();
    for(;;) loop();
}

void EXTI15_10_IRQHandler(void) {
    //Pulsante X
    if(EXTI_isset(EXTI10)) {
        if(current_status != SETUP) {
            current_status = SETUP;
            GPIO_write(GPIOC, 3, 1);
        }
        else if(current_status == SETUP) {
            current_status = READY;
            actual_SS = setted_SS;
            actual_CC = setted_CC;
            GPIO_write(GPIOC, 3, 0);
        }

        EXTI_clear(EXTI10);
    }
}
void EXTI4_IRQHandler(void) {
    //Pulsante Y
    if(EXTI_isset(EXTI4)) {
        if(current_status == READY) {
            current_status = WORKING;
        }
        else if(current_status == WORKING) {
            current_status = PAUSED;
        } 
        else if(current_status == PAUSED) {
            current_status = WORKING;
        }
        EXTI_clear(EXTI4);
    }
}
void EXTI9_5_IRQHandler(void) {
    //Pulsante Z
    if(EXTI_isset(EXTI5)) {
        if(current_status == SETUP) {
            if(setted_SS > 0) {
                setted_SS--;
            }
        }
        else if(current_status == PAUSED) {
            GPIO_write(GPIOC, 2, 0);
            current_status = SETUP;
        }

        EXTI_clear(EXTI5);
    }

    //Pulsante T
    if(EXTI_isset(EXTI6)) {
        if(current_status == SETUP) {
            if(setted_SS < 99) {
                setted_SS++;
            }
        }
        EXTI_clear(EXTI6);
    }
}

void TIM2_IRQHandler(void) {
    if(TIM_update_check(TIM2)) { //Ogni 0.01 s
        if(current_status == SETUP) {
            sprintf(to_display, "%02d%02d", setted_SS, setted_CC);
        }
        else if(current_status == READY) {
            sprintf(to_display, "%02d%02d", actual_SS, actual_CC);
        }
        else if(current_status == WORKING) {
            yellow_flash_counter++;
            if(yellow_flash_counter >= 25) {
                GPIO_toggle(GPIOC, 2);
                yellow_flash_counter = 0;

            }
            sprintf(to_display, "%02d%02d", actual_SS, actual_CC);
            if(actual_SS > 0) {
                if(actual_CC == 0) {
                    actual_SS--;
                    actual_CC = 99;
                }
                actual_CC--;
            }
            else if(actual_SS == 0 && actual_CC > 0) {
                actual_CC--;
            }
            else if(actual_SS == 0 && actual_CC == 0) {
                current_status = FINISHED;
            }
        }
        else if(current_status == PAUSED) {
            
        }
        else if(current_status == FINISHED) {
            GPIO_write(GPIOC, 2, 0);
        }


        TIM_update_clear(TIM2);
    }
}