#include "buzzer.h"
#include "config.h"
#include "gpio.h"
#include "systick.h"
#include "regmap-int.h"

#define INITIAL_TEMESTAMP 0
#define DEFAULT_DURATION 500 // sound duration in ms
#define DEFAULT_PERIOD 250000 // period in PWM, it is used when configuring TIM3
#define DEFAULT_CYCLE 125000 // duty cycle in PWM, it is used when configuring TIM3, 

static const gpio_pin_t buzzer_gpio = { EC_GPIO_BUZZER };

struct buzzer_ctx {
    bool is_enabled; // false - buzzer disabled, true - enabled
    bool done; // false - buzzer hasn't worked yet when power on, true - worked
    systime_t duration;
    uint32_t period;
    uint32_t duty_cycle;
};
// In this case buzzer_ctx is filled with the default values, but it can be filled with values saved in the flash
static struct buzzer_ctx buzzer_ctx = {
        .done = false,
        .is_enabled = true,
        .duration = DEFAULT_DURATION,
        .period = DEFAULT_PERIOD,
        .duty_cycle = DEFAULT_CYCLE,
};

void buzzer_gpio_init(void) {
    GPIO_S_SET_PUSHPULL(buzzer_gpio);
    GPIO_S_SET_AF(buzzer_gpio, TIM3_CH2);
}

void tim3_init_and_config() {
    RCC->APBENR1 |= RCC_APBENR1_TIM3EN; // Enable clocking TIM3
    // This code is supposed to configure TIM3 channel 2 for PWM
}

void buzzer_init(void) {
    tim3_init_and_config();
    buzzer_gpio_init();
}

void buzzer_deinit(void) {
    RCC->APBENR1 &= ~RCC_APBENR1_TIM3EN; // Disable clocking TIM3
    GPIO_S_RESET(buzzer_gpio);
}

static inline void buzzer_start(void) {
	TIM3->CR1 |= TIM_CR1_CEN; // TIM3 ON
}

static inline void buzzer_stop(void) {
	TIM3->CR1 &= ~TIM_CR1_CEN; // TIM3 OFF
}

bool is_buzzer_enabled(void) {
	return buzzer_ctx.is_enabled;
}

bool buzzer_done(void) {
    return buzzer_ctx.done;
}

void buzzer_initial_buzz(void) {
    buzzer_start();
    systime_t delay = buzzer_ctx.duration;
    if (systick_get_time_since_timestamp(INITIAL_TEMESTAMP) < delay)
        return;
    buzzer_stop();
    buzzer_deinit();
    buzzer_ctx.done = true;
}

void buzzer_do_periodic_work(void) {
    if (regmap_is_region_changed(REGMAP_REGION_BUZZER)) {
        struct REGMAP_BUZZER b;
        regmap_get_region_data(REGMAP_REGION_BUZZER, &b, sizeof(b));
        if (buzzer_ctx.is_enabled != b.is_enabled)
            buzzer_ctx.is_enabled = b.is_enabled;
        if (buzzer_ctx.duration != b.duration)
            buzzer_ctx.duration = b.duration;
        if (buzzer_ctx.period != b.period)
            buzzer_ctx.period = b.period;
        if (buzzer_ctx.duty_cycle != b.duty_cycle)
            buzzer_ctx.duty_cycle = b.duty_cycle;

        regmap_clear_changed(REGMAP_REGION_BUZZER);
    }
}