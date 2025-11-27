#include "stm32_unict_lib.h"
#include <stdio.h>

typedef enum {
    SET_TARGET,
    RUN
} mode;

typedef struct {
    double x;
    double y;
} Point;

Point current_point;
Point target_point;

mode current_mode = SET_TARGET;
char to_display[5];

int move_counter = 0;

void setup() {
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

    ADC_init(ADC1, ADC_RES_8, ADC_ALIGN_RIGHT);
    ADC_channel_config(ADC1, GPIOC, 0, 10);
    ADC_channel_config(ADC1, GPIOC, 1, 11);
    ADC_on(ADC1);

    TIM_init(TIM2);
    TIM_config_timebase(TIM2, 42000, 2); // 0.001 s (1 ms)
    TIM_enable_irq(TIM2, IRQ_UPDATE);
    TIM_on(TIM2);

    current_point.x = 0;
    current_point.y = 0;

    target_point.x = 0;
    target_point.y = 0;

    GPIO_write(GPIOC, 2, 1);
}

void loop() {
    DISPLAY_puts(0, to_display);
}

int main() {
    setup();
    for(;;) loop();
}

void EXTI15_10_IRQHandler() {
    //Pulsante X
    if(EXTI_isset(EXTI10)) {
        if(current_mode != SET_TARGET) {
            GPIO_write(GPIOB, 0, 0);
            GPIO_write(GPIOC, 2, 1);
            printf("Passaggio alla modalità SET TARGET\n");
            current_mode = SET_TARGET;
        }
        EXTI_clear(EXTI10);
    }
}
void EXTI4_IRQHandler() {
    //Pulsante Y
    if(EXTI_isset(EXTI4)) {
        if(current_mode == SET_TARGET) {
            GPIO_write(GPIOB, 0, 1);
            GPIO_write(GPIOC, 2, 0);
            printf("Passaggio alla modalità RUN\n");
            current_mode = RUN;
        }
        EXTI_clear(EXTI4);
    }
}
void EXTI9_5_IRQHandler() {
    //Pulsante Z
    if(EXTI_isset(EXTI5)) {
        target_point.x = 0;
        target_point.y = 0;
        printf("Posizione target resettata a (0,0)\n");

        EXTI_clear(EXTI5);
    }
}

void TIM2_IRQHandler() {
    if(TIM_update_check(TIM2)) { //Ogni 0.001 s (1 ms)
        if(current_mode == SET_TARGET) {
            GPIO_write(GPIOB, 8, 1);
            //Campionamento punto x tramite trimmer AN10
            ADC_sample_channel(ADC1, 10);
            ADC_start(ADC1);
            while(!ADC_completed(ADC1)) {}
            target_point.x = ((ADC_read(ADC1) * 99) / 255);
            //Campionamento punto y tramite trimmer AN11
            ADC_sample_channel(ADC1, 11);
            ADC_start(ADC1);
            while(!ADC_completed(ADC1)) {}
            target_point.y = ((ADC_read(ADC1) * 99) / 255);
            sprintf(to_display, "%02d%02d", (int)target_point.x, (int)target_point.y);
        }
        else if(current_mode == RUN) {
            move_counter++;
            if(move_counter >= 1) {
                if(current_point.x < target_point.x) {
                        current_point.x += 0.005;    
                }
                if(current_point.y < target_point.y) {
                        current_point.y += 0.005;
                }
                if(current_point.x > target_point.x) {
                        current_point.x -= 0.005;    
                }
                if(current_point.y > target_point.y) {
                        current_point.y -= 0.005;
                }
                sprintf(to_display, "%02d%02d", (int)current_point.x, (int)current_point.y);
                move_counter = 0;
            }
        }

        TIM_update_clear(TIM2);
    }
}