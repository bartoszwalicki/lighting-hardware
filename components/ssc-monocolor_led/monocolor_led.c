#include "monocolor_led.h"

static xQueueHandle* buttonQueueHandle = NULL;
TaskHandle_t handleEventFromQueueTaskHandler = NULL;
static uint8_t ledState = 0;
static const char* TAG = "MonocolorLED";

static void handleEventFromQueue(void* arg) {
    uint8_t channelNumber;
    while(1) {
        if(xQueueReceive(*buttonQueueHandle, &channelNumber, portMAX_DELAY)) {
            ESP_LOGI(TAG,"Changing state of channel %d to %d", channelNumber, !ledState);
            if(!ledState) {
                ledc_set_fade_time_and_start(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 4095, 250,LEDC_FADE_NO_WAIT);
            } else {
                ledc_set_fade_time_and_start(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0, 250,LEDC_FADE_NO_WAIT);
            }
            ledState = !ledState;
        }

        vTaskDelay(50/ portTICK_RATE_MS);
    }
};

void initLeds(xQueueHandle* queueHandler) {
    buttonQueueHandle = queueHandler;

    ledc_fade_func_install(0);

    xTaskCreate(handleEventFromQueue,"handleEventFromQueue", 2048, NULL, tskIDLE_PRIORITY, &handleEventFromQueueTaskHandler);
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
}