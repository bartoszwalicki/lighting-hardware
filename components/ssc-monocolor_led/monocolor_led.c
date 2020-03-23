#include "monocolor_led.h"

static xQueueHandle* buttonQueueHandle = NULL;
TaskHandle_t handleEventFromQueueTaskHandler = NULL;
static const char* TAG = "MonocolorLED";

static void handleEventFromQueue(void* arg) {
    uint8_t ledState = 0;
    uint8_t inputGpioNumber;
    uint8_t ledcChannel = 0;

    while(1) {
        if(xQueueReceive(*buttonQueueHandle, &inputGpioNumber, portMAX_DELAY)) {
            struct ChannelGpioMap* ptr = channelGpioMap;

            for (size_t i = 0; i < gpioMapSize; i++, ptr++)
            {
                if(ptr->inputGpioPin == inputGpioNumber) {
                    ESP_LOGI(TAG,"Changing state of channel %d to %d to target duty of %d \r", ptr->ledcChannel, !ptr->currentState, ptr->targetDuty);

                    if(!ptr->currentState) {
                        ledc_set_fade_time_and_start(LEDC_HIGH_SPEED_MODE, ptr->ledcChannel, ptr->targetDuty, 250,LEDC_FADE_NO_WAIT);
                    } else {
                        ledc_set_fade_time_and_start(LEDC_HIGH_SPEED_MODE, ptr->ledcChannel, 0, 250,LEDC_FADE_NO_WAIT);
                    }
                    ptr->currentState = !(ptr->currentState);
                }
            }
        }

        vTaskDelay(50/ portTICK_RATE_MS);
    }
};

void initLeds(xQueueHandle* queueHandler) {
    buttonQueueHandle = queueHandler;

    ledc_fade_func_install(0);

    xTaskCreate(handleEventFromQueue,"handleEventFromQueue", 2048, NULL, tskIDLE_PRIORITY, &handleEventFromQueueTaskHandler);
}

void addChannel(struct ChannelGpioMap* channelConfig) {
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_12_BIT,
        .freq_hz = 16000,                     
        .speed_mode = LEDC_HIGH_SPEED_MODE,    
        .timer_num = LEDC_TIMER_0
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .channel = channelConfig->ledcChannel,
        .duty = 0,
        .gpio_num = channelConfig->outputLedChannelPin,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0
    };

    ledc_channel_config(&ledc_channel);
}