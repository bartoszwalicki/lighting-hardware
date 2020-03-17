#include "leds.h"

void initLeds(void) {
    ledc_fade_func_install(0);
}

void addChannel(uint8_t gpio) {
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_12_BIT,
        .freq_hz = 16000,                     
        .speed_mode = LEDC_HIGH_SPEED_MODE,    
        .timer_num = LEDC_TIMER_0
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL_0,
        .duty = 0,
        .gpio_num = gpio,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0
    };

    ledc_channel_config(&ledc_channel);

    while(1) {
        ledc_set_fade_time_and_start(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0, 3000,LEDC_FADE_NO_WAIT);
        vTaskDelay(5000/portTICK_PERIOD_MS);
        ledc_set_fade_time_and_start(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 4095, 3000,LEDC_FADE_NO_WAIT);
        vTaskDelay(5000/portTICK_PERIOD_MS);
    }
}