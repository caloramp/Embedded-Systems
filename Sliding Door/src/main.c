#include "stm32_unict_lib.h"
#include <stdio.h>
#include <string.h>

typedef enum {
    WORKING,
    SETTINGS
} functionality;

typedef enum {
    AUTO,
    MANUAL,
} mode;

typedef enum {
    CLOSED,
    OPENING,
    WAITING,
    OPENED,
    CLOSING
} status_t;

typedef enum {
    NONE,
    X,
    Y
} conf_t;

functionality current_functionality = WORKING;
mode current_mode = AUTO;
status_t current_status = CLOSED;
conf_t conf_option = NONE;

int Ta = 10;
int opening_counter = 0;
int waiting_counter = 0;
int closing_counter = 0;

int flashing_counter = 0;

char *door[] = {
    "----",
    "--- ",
    "--  ",
    "-   ",
    "    ",
};

int door_pos = 0;

char to_display[5];

void setup(void) {
    ClockConfig();
    DISPLAY_init();
    CONSOLE_init();

    GPIO_init(GPIOB);
    GPIO_init(GPIOC);

    GPIO_config_output(GPIOB, 0);
    GPIO_config_output(GPIOC, 2);
    GPIO_config_output(GPIOC, 3);

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
    TIM_config_timebase(TIM2, 42000, 200); //0.1s
    TIM_enable_irq(TIM2, IRQ_UPDATE);
    TIM_on(TIM2);

    TIM_init(TIM3);
    TIM_config_timebase(TIM3, 42000, 20); //0.01s
    TIM_enable_irq(TIM3, IRQ_UPDATE);
    TIM_on(TIM3);

    ADC_init(ADC1, ADC_RES_12, ADC_ALIGN_RIGHT);
    ADC_channel_config(ADC1, GPIOC, 1, 11);
    ADC_on(ADC1);
    ADC_sample_channel(ADC1, 11);

    strcpy(to_display, door[door_pos]);
}

void loop(void) {
    DISPLAY_puts(0, to_display);
}

void EXTI15_10_IRQHandler(void) {
    //Pushbutton X
    if(EXTI_isset(EXTI10)) {

        if(current_functionality == WORKING) {
            if(current_mode == AUTO) { //Fotocellula
                if(current_status == CLOSED || current_status == CLOSING) {
                    current_status = OPENING;
                }
                else if(current_status == WAITING) {
                    waiting_counter = 0;
                }
            }
        }
        else if(current_functionality == SETTINGS) {
            conf_option = X;
        }

        EXTI_clear(EXTI10);
    }
}

void EXTI4_IRQHandler(void) {
    //Pushbutton Y
    if(EXTI_isset(EXTI4)) {
        if(current_functionality == WORKING) {
            if(current_mode == AUTO) { //Fotocellula
                if(current_status == CLOSED || current_status == CLOSING) {
                    current_status = OPENING;
                }
                else if(current_status == WAITING) {
                    waiting_counter = 0;
                }
            }
        }
        else if(current_functionality == SETTINGS) { // Impostazione AUTO/MANUAL
            conf_option = Y;
            if(current_mode == AUTO) {
                current_mode = MANUAL;
            }
            else if(current_mode == MANUAL) {
                current_mode = AUTO;
            }
        }
        EXTI_clear(EXTI4);
    }
}

void EXTI9_5_IRQHandler(void) {
    //Pushbutton Z
    if(EXTI_isset(EXTI5)) {
        if(current_functionality == WORKING) {
            if(current_mode == MANUAL) { //Comando manuale
                if(current_status == CLOSED) {
                    current_status = OPENING;
                }
                else if(current_status == OPENED) {
                    current_status = CLOSING;
                }
            }
        }
        else if(current_functionality == SETTINGS) {

        }

        EXTI_clear(EXTI5);
    }

    //Pushbutton T
    if(EXTI_isset(EXTI6)) {
        if(current_functionality != SETTINGS) {
            current_functionality = SETTINGS;
        }
        else {
            current_functionality = WORKING;
        }
        EXTI_clear(EXTI6);
    }
}

void TIM2_IRQHandler(void) {
    if(TIM_update_check(TIM2)) { //Ogni 0.1s
        if(current_functionality == WORKING) {
            GPIO_write(GPIOB, 0, 0);
            strcpy(to_display, door[door_pos]);
            if(current_mode == AUTO) {
                GPIO_write(GPIOC, 2, 0);
                if(current_status == OPENING) {
                    printf("Stato corrente: OPENING\n");
                    opening_counter++;
                    if (opening_counter >= 2) {
                        if (door_pos < 4) {
                            door_pos++;
                            strcpy(to_display, door[door_pos]);
                        } else {
                            current_status = WAITING;
                        }
                        opening_counter = 0;
                    }
                }
                else if(current_status == WAITING) {
                    GPIO_write(GPIOC, 3, 1);
                    printf("Stato corrente: WAITING\n");
                    waiting_counter++;
                    if(waiting_counter >= Ta) {
                        waiting_counter = 0;
                        current_status = CLOSING;
                    }
                }
                else if(current_status == CLOSING) {
                    printf("Stato corrente: CLOSING\n");
                    closing_counter++;
                    if(closing_counter >= 2) {
                        if(door_pos > 0) {
                            door_pos--;
                            strcpy(to_display, door[door_pos]);
                        }
                        if(door_pos == 0) {
                            GPIO_write(GPIOC, 3, 0);
                            current_status = CLOSED;
                            printf("Stato corrente: CLOSED\n");
                        }
                        closing_counter = 0;
                    }
                }
            }
            else if(current_mode == MANUAL) {
                GPIO_write(GPIOC, 2, 1);
                if(current_status == OPENING) {
                    opening_counter++;
                    if (opening_counter >= 2) {
                        if (door_pos < 4) {
                            door_pos++;
                            strcpy(to_display, door[door_pos]);
                        } else {
                            current_status = OPENED;
                        }
                        opening_counter = 0;
                    }
                }
                else if(current_status == CLOSING) {
                    closing_counter++;
                    if(closing_counter >= 2) {
                        if(door_pos > 0) {
                            door_pos--;
                            strcpy(to_display, door[door_pos]);
                        }
                        if(door_pos == 0) {
                            current_status = CLOSED;
                            printf("Stato corrente: CLOSED\n");
                        }
                        closing_counter = 0;
                    }
                }
                if(current_status == OPENED) {
                    GPIO_write(GPIOC, 3, 1);
                }
                if(current_status == CLOSED) {
                    GPIO_write(GPIOC, 3, 0);
                }
            }
        }
        else if(current_functionality == SETTINGS) {
            GPIO_write(GPIOB, 0, 1);
            if(conf_option == NONE) {
                strcpy(to_display, "conf");
            }
            else if(conf_option == X) {
                ADC_start(ADC1);
                while(!ADC_completed(ADC1)) {}
                Ta = (10 + ((ADC_read(ADC1) * 20) / 4095));
                sprintf(to_display, "%4d", Ta);
            }
            else if(conf_option == Y) {
                if(current_mode == AUTO) {
                    strcpy(to_display, "Auto");
                }
                else if(current_mode == MANUAL) {
                    strcpy(to_display, "Hand");
                }
            }
        }
        
        TIM_update_clear(TIM2);
    }
}

void TIM3_IRQHandler(void) {
    if(TIM_update_check(TIM3)) {
        if(current_status == OPENING || current_status == CLOSING) {
            flashing_counter++;
            if(flashing_counter >= 5) {
                GPIO_toggle(GPIOC, 3);
                flashing_counter = 0;
            }
        }
        TIM_update_clear(TIM3);
    }
}


int main() {
    setup();
    for(;;) loop();
}