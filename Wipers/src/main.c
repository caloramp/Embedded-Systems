#include "stm32_unict_lib.h"
#include <stdlib.h>

typedef enum {
    OFF,
    RAIN,
    TIMED,
    CONTINUE,
    CONFIGURATION
} mode_t;

mode_t current_mode = OFF;

int rain_dropped = 0;
int cumulated_rain = 0;

int TS1 = 20;
int TS2 = 5;

int counter = 0;

int wipers_pos = 0;
int wipers_interval = 0;
int timed_counter = 0;

char *wipers[] = {
    "-   ",
    " -  ",
    "  - ",
    "   -",
    "  - ",
    " -  ",
    "-   "
};


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
    ADC_channel_config(ADC1, GPIOC, 0, 10);
    ADC_channel_config(ADC1, GPIOC, 1, 11);
    //ADC_sample_channel(ADC1, 10);
    ADC_on(ADC1);

    TIM_init(TIM2);
    TIM_config_timebase(TIM2, 42000, 200); // 0.1 s
    TIM_enable_irq(TIM2, IRQ_UPDATE);
    TIM_on(TIM2);
}

void loop() {
    if(current_mode != CONFIGURATION) {
        DISPLAY_puts(0, wipers[wipers_pos]);
    }
    else {
        DISPLAY_puts(0, "conf");
        if(kbhit()) {
            char m;
            char c = readchar();
            if(c == '1') {
                printf("Premi 1 per impostare Ts1 a 2 s\n");
                printf("Premi 2 per impostare Ts1 a 2.5 s\n");
                printf("Premi 3 per impostare Ts1 a 3 s\n");
                if(kbhit()) {
                    m = readchar();
                    if(m == '1') {
                        TS1 = 20;
                        printf("Ts1 impostato a 2 s");
                    }
                    if(m == '2') {
                        TS1 = 25;
                        printf("Ts1 impostato a 2.5 s");
                    }
                    if(m == '3') {
                        TS1 = 30;
                        printf("Ts1 impostato a 3 s");
                    }
                }
            }
            if(c == '2') {
                printf("Premi 1 per impostare Ts2 a 0.5\n");
                printf("Premi 2 per impostare Ts2 a 1\n");
                printf("Premi 3 per impostare Ts2 a 1.5\n");
                if(kbhit()) {
                    m = readchar();
                    if(m == '1') {
                        TS2 = 5;
                        printf("Ts2 impostato a 0.5 s");
                    }
                    if(m == '2') {
                        TS2 = 10;
                        printf("Ts2 impostato a 1 s");
                    }
                    if(m == '3') {
                        TS2 = 1.5;
                        printf("Ts2 impostato a 1.5 s");
                    }
                }
            }
            if(c == 'e' || c == 'E') {
                current_mode = OFF;
            }
        }
    }
}

int main() {
    setup();
    for(;;) loop();
}

void EXTI15_10_IRQHandler(void) {
    //Pushbutton X
    if(EXTI_isset(EXTI10)) {
        if(current_mode != RAIN) {
            current_mode = RAIN;
            GPIO_write(GPIOB, 0, 1); // Accensione LED rosso
            GPIO_write(GPIOC, 2, 0); // Spegnimento LED giallo
            GPIO_write(GPIOC, 3, 0); // Spegnimento LED verde
        }
        else {
            current_mode = OFF;
            GPIO_write(GPIOB, 0, 0); // Spegnimento LED rosso

        }
        EXTI_clear(EXTI10);
    }
}

void EXTI4_IRQHandler(void) {
    //Pushbutton Y
    if(EXTI_isset(EXTI4)) {
        if(current_mode != TIMED) {
            current_mode = TIMED;
            GPIO_write(GPIOC, 2, 1); // Accensione LED giallo
            GPIO_write(GPIOB, 0, 0); // Spegnimento LED rosso
            GPIO_write(GPIOC, 3, 0); // Spegnimento LED verde
        }
        else {
            current_mode = OFF;
            GPIO_write(GPIOC, 2, 0); // Spegnimento LED giallo
        }
        EXTI_clear(EXTI4);
    }
}

void EXTI9_5_IRQHandler(void) {
    //Pushbutton Z
    if(EXTI_isset(EXTI5)) {
        if(current_mode != CONTINUE) {
            current_mode = CONTINUE;
            GPIO_write(GPIOC, 3, 1); // Accensione LED verde
            GPIO_write(GPIOB, 0, 0); // Spegnimento LED rosso
            GPIO_write(GPIOC, 2, 0); // Spegnimento LED giallo
        }
        else {
            current_mode = OFF;
            GPIO_write(GPIOC, 3, 1); // Spegnimento LED verde
        }
        EXTI_clear(EXTI5);
    }

    //Pushbutton T
    if(EXTI_isset(EXTI6)) {
        if(current_mode != CONFIGURATION) {
            current_mode = CONFIGURATION;
            GPIO_write(GPIOB, 0, 1);
            GPIO_write(GPIOC, 2, 1);
            GPIO_write(GPIOC, 3, 1);

            printf("Modalità configurazione:\n");
            printf("\t-Per configurare Ts1 premi il pulsante 1\n");
            printf("\t-Per configurare Ts2 premi il pulsante 2\n");
            printf("\t-Per uscire dal menù premi il pulsante e\n");
        }
        else {
            current_mode = OFF;
            GPIO_write(GPIOB, 0, 0);
            GPIO_write(GPIOC, 2, 0);
            GPIO_write(GPIOC, 3, 0);
        }
        EXTI_clear(EXTI6);
    }
}

void TIM2_IRQHandler(void) {
    if(TIM_update_check(TIM2)) { // Ogni 0.1 s
        ++counter;
        if(current_mode == RAIN) {
            ADC_sample_channel(ADC1, 11);
            ADC_start(ADC1);
            while (!ADC_completed(ADC1)) {}
            rain_dropped = ((ADC_read(ADC1) * 5) / 4095);
            cumulated_rain += rain_dropped;

            if(cumulated_rain >= 100) {
                //++counter;
                if(counter < TS1) { // Ogni TS1 secondi
                    wipers_pos++;
                    if(wipers_pos == 7) {
                        wipers_pos = 0;
                        cumulated_rain = 0;
                    }
                }
            }            
            //counter = 0;
        }
        else if(current_mode == TIMED) {
            ADC_sample_channel(ADC1, 10);
            ADC_start(ADC1);
            while(!ADC_completed(ADC1)) {}
            wipers_interval = (1 + ((ADC_read(ADC1) * 4) / 4095));

            timed_counter++;
            if(timed_counter >= (wipers_interval * 10)) {
                if(counter >= TS1) { // Ogni TS1 s
                    wipers_pos++;
                    if(wipers_pos == 7) {
                        wipers_pos = 0;
                        timed_counter = 0;
                    }
                }
            }
        }
        else if(current_mode == CONTINUE) {
            if(counter < TS2) { // Ogni TS2 s
                wipers_pos++;
                if(wipers_pos == 7) {
                    wipers_pos = 0;
                }
            }
        }
        counter = 0;
        TIM_update_clear(TIM2);
    }
}