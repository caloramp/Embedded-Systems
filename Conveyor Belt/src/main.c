#include "stm32_unict_lib.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

typedef enum {
    RED,
    YELLOW,
    GREEN
} status_t;

status_t current_status = RED;

int arrival_speed = 5;
char arr_s;

int red_pack = 0;
int yellow_pack = 0;
int green_pack = 0;

int rand_value;

char to_display[5];

int counter = 0;
int flash_counter = 0;

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

    GPIO_config_output(GPIOB, 0);
    GPIO_config_output(GPIOC, 2);
    GPIO_config_output(GPIOC, 3);

    ADC_init(ADC1, ADC_RES_12, ADC_ALIGN_RIGHT);
    ADC_channel_config(ADC1, GPIOC, 1, 11);
    ADC_sample_channel(ADC1, 11);
    ADC_on(ADC1);

    TIM_init(TIM2);
    TIM_config_timebase(TIM2, 42000, 200); //0.1s
    TIM_enable_irq(TIM2, IRQ_UPDATE);
    TIM_on(TIM2);

    srand(time(0));
}

void loop(void) {
    DISPLAY_puts(0, to_display);
}

int main() {
    setup();
    for(;;) loop();
}

void EXTI15_10_IRQHandler(void) {
    //Pushbutton X
    if(EXTI_isset(EXTI10)) {
        if(current_status != RED) {
            current_status = RED;
            sprintf(to_display, "r %2d", red_pack);
            printf("[X] Sul display verrà mostrato il numero di pacchi rossi\n");
        }
        EXTI_clear(EXTI10);
    }
}

void EXTI4_IRQHandler(void) {
    //Pushbutton Y
    if(EXTI_isset(EXTI4)) {
        if(current_status != YELLOW) {
            current_status = GREEN;
            sprintf(to_display, "y %2d", yellow_pack);
            printf("[Y] Sul display verrà mostrato il numero di pacchi gialli\n");
        }
        EXTI_clear(EXTI4);
    }
}

void EXTI9_5_IRQHandler(void) {
    //Pushbutton Z
    if(EXTI_isset(EXTI5)) {
        if(current_status != GREEN) {
            current_status = GREEN;
            sprintf(to_display, "g %2d", green_pack);
            printf("[Z] Sul display verrà mostrato il numero di pacchi verdi\n");
        }
        
        EXTI_clear(EXTI5);
    }

    //Pushbutton T
    if(EXTI_isset(EXTI6)) {
        red_pack = 0;
        yellow_pack = 0;
        green_pack = 0;
        printf("[T] I contatori sono stati azzerati\n");

        EXTI_clear(EXTI6);
    }
}

void TIM2_IRQHandler(void) {
    if(TIM_update_check(TIM2)) {
        ADC_start(ADC1);
        while(!ADC_completed(ADC1)) {}
        arrival_speed = (5 + ((ADC_read(ADC1) * 15) / 4095)); // Range [0.5, 2]

        counter++;
        if(arrival_speed < counter) {
            rand_value = rand() % 3;
            if(rand_value == 0) {
                red_pack++;
                GPIO_write(GPIOB, 0, 1);
                GPIO_write(GPIOB, 0, 0);
                printf("Pacco rosso in arrivo\n");
                sprintf(to_display, "r %2d", red_pack);
                rand_value = 3;
                
            } else if(rand_value == 1) {
                yellow_pack++;
                GPIO_write(GPIOC, 2, 1);
                GPIO_write(GPIOC, 2, 0);
                printf("Pacco giallo in arrivo\n");
                sprintf(to_display, "y %2d", yellow_pack);
                rand_value = 3;
            }
            else if (rand_value == 2) {
                green_pack++;
                GPIO_write(GPIOC, 3, 1);
                GPIO_write(GPIOC, 3, 0);
                printf("Pacco verde in arrivo\n");
                sprintf(to_display, "g %2d", green_pack);
                rand_value = 3;
            }
            counter = 0;
        }
        TIM_update_clear(TIM2);
    }
}
