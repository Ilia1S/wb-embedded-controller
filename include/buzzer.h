#pragma once
#include <stdint.h>
#include <stdbool.h>

void buzzer_gpio_init(void);
void tim3_init_and_config();
void buzzer_init(void);
void buzzer_deinit(void);
bool is_buzzer_enabled(void);
bool buzzer_done(void);
void buzzer_initial_buzz(void);
void buzzer_do_periodic_work(void);
