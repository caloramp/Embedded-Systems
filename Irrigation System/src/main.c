#include "stm32_unict_lib.h"
#include <stdio.h>
#include <time.h>

typedef enum {
    NORMAL,
    CONFIGURATION
} mode;

mode current_mode = NORMAL;
time_t now;
struct tm *t;

char to_display[5];

void setup(void) {
    ClockConfig();
    DISPLAY_init();
    CONSOLE_init();

    GPIO_init(GPIOB);
    GPIO_init(GPIOC);

    GPIO_config_input(GPIOB, 10);
    GPIO_config_input(GPIOB, 4);
    GPIO_config_input(GPIOB, 5);

    GPIO_config_EXTI(GPIOB, EXTI10);
    GPIO_config_EXTI(GPIOB, EXTI4);
    GPIO_config_EXTI(GPIOB, EXTI5);

    EXTI_enable(EXTI10, FALLING_EDGE);
    EXTI_enable(EXTI4, FALLING_EDGE);
    EXTI_enable(EXTI5, FALLING_EDGE);
    
    GPIO_config_output(GPIOB, 0);
    GPIO_config_output(GPIOB, 8);
    GPIO_config_output(GPIOC, 2);

    ADC_init(ADC1, ADC_RES_12, ADC_ALIGN_RIGHT);
    ADC_channel_config(ADC1, GPIOC, 0, 10);
    ADC_channel_config(ADC1, GPIOC, 1, 11);
    ADC_on(ADC1);

    TIM_init(TIM2);
    TIM_config_timebase(TIM2, 8400, 10000); //1 sec
    TIM_enable_irq(TIM2, IRQ_UPDATE);
    TIM_on(TIM2);
}

void loop(void) {
    if(current_mode == NORMAL) {
        GPIO_write(GPIOB, 8, 1);
    }
    DISPLAY_puts(0, to_display);
}

int main() {
    setup();
    for(;;) loop();
}

void EXTI15_10_IRQHandler(void) {

}

void TIM2_IRQHandler(void) {
    if(TIM_update_check(TIM2)) { //Ogni secondo
        now = time(NULL);
        t = localtime(&now);
        sprintf(to_display, "%02d%02d", t->tm_min, t->tm_sec);
        printf("%02d:%02d", t->tm_min, t->tm_sec);

        TIM_update_clear(TIM2);
    }
}
