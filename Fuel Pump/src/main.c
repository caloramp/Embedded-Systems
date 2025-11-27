#include "stm32_unict_lib.h"
#include <stdio.h>

typedef enum {
    STARTING,
    EROGATION,
    FINISHING
} status_t;

status_t status = STARTING;

float fuel_price = 1.5;
int amount = 0;
float to_be_erogated;
float erogated = 0;

int counter = 0;
int temp = 0;

int yellow_flashing_counter = 0;

char to_be_displayed[5];

void flashing_yellow_led() {
    ++yellow_flashing_counter;
    if(yellow_flashing_counter >= 5) {
        GPIO_toggle(GPIOC, 2);
        yellow_flashing_counter = 0;
    }
}

void set_fuel_quantity() {
    to_be_erogated = amount / fuel_price; 
}

void setup(void) {
    ClockConfig();
    DISPLAY_init();

    GPIO_init(GPIOB);
    GPIO_init(GPIOC);

    GPIO_config_input(GPIOB, 10); // X
    GPIO_config_input(GPIOB, 4); //  Y
    GPIO_config_input(GPIOB, 5); //  Z
    GPIO_config_input(GPIOB, 6); //  T

    GPIO_config_output(GPIOB, 0); // LED rosso
    GPIO_config_output(GPIOC, 2); // LED giallo
    
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
    
    TIM_init(TIM3);
    TIM_config_timebase(TIM3, 42000, 200); // 0.1 s
    TIM_enable_irq(TIM3, IRQ_UPDATE);
    TIM_on(TIM3);
}

void loop(void) {
    if(status == STARTING) {
        DISPLAY_puts(0, to_be_displayed);
        DISPLAY_dp(1, 0);
    }
    else if(status == EROGATION) {
        DISPLAY_puts(0, to_be_displayed);
        DISPLAY_dp(1, 1);
    }
}

void EXTI15_10_IRQHandler(void) {
    //Pushbutton X
    if(EXTI_isset(EXTI10)) {
        if(status == STARTING) {
            amount += 20;
        }
        EXTI_clear(EXTI10);
    }
}

void EXTI4_IRQHandler(void) {
    //Pushbutton Y
    if(EXTI_isset(EXTI4)) {
        if(status == STARTING) {
            amount += 5;
        }
        EXTI_clear(EXTI4);
    }
}

void EXTI9_5_IRQHandler(void) {
    //Pushbutton Z
    if(EXTI_isset(EXTI5)) {
        if(status == STARTING) {
            amount = 0;
        }
        EXTI_clear(EXTI5);
    }

    //Pushbutton T
    if(EXTI_isset(EXTI6)) {
        if(status == STARTING) {
            status = EROGATION;
            set_fuel_quantity();
        }
        else if(status == FINISHING) {
            status = STARTING;
        }

        EXTI_clear(EXTI6);
    }
}

void TIM2_IRQHandler(void) {
    if(TIM_update_check(TIM2)) { // Ogni 0.1 s
        if(status == STARTING) {
            GPIO_write(GPIOB, 0, 0);
            sprintf(to_be_displayed, "%4d", amount);
        }
        else if(status == EROGATION) {
            if(erogated <= to_be_erogated) {
                temp = erogated * 100;
                sprintf(to_be_displayed, "%4d", temp);
                erogated += 0.05;
            }
            else {
                status = FINISHING;
                amount = 0;
            }
        }
        else {
            GPIO_write(GPIOB, 0, 1);
            GPIO_write(GPIOC, 2, 0);
            temp = 0;
        }
        TIM_update_clear(TIM2);
    }

}

void TIM3_IRQHandler(void) {
    if(TIM_update_check(TIM3)) { // 
        if(status == EROGATION) {
            flashing_yellow_led();
        }
        TIM_update_clear(TIM3);
    }

}

int main() {
    setup();
    for(;;) loop();
}