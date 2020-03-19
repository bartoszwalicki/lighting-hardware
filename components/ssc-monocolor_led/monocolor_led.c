#include "monocolor_led.h"

static xQueueHandle* buttonQueueHandle = NULL;
TaskHandle_t handleEventFromQueueTaskHandler = NULL;
static uint8_t ledState = 0;
static const char* TAG = "MonocolorLED";
static struct ChannelGpioMap** channelGpioMap = NULL;
static uint8_t* mapSize = NULL;

static void handleEventFromQueue(void* arg) {
    uint8_t inputGpioNumber;
    while(1) {
        if(xQueueReceive(*buttonQueueHandle, &inputGpioNumber, portMAX_DELAY)) {
            ESP_LOGI(TAG,"Changing state of channel %d to %d", inputGpioNumber, !ledState);
            // lookupLedcChannel(&inputGpioNumber);
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

void initLeds(xQueueHandle* queueHandler, struct ChannelGpioMap** map, const uint8_t* mapSize) {
    buttonQueueHandle = queueHandler;
    channelGpioMap = map;
    mapSize = mapSize;

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

// void lookupLedcChannel(uint8_t* gpioPin) {
//     struct ChannelGpioMap* ptr = channelGpioMap;
//     for (size_t i = 0; i < *mapSize; i++, ptr++)
//     {
//         if(ptr->inputGpioPin == *gpioPin) {
//             printf("Data: %d %d", ptr->inputGpioPin, ptr->outputLedChannelPin);
//         }
//     }
// }