#include "monocolor_led.h"

static xQueueHandle* buttonQueueHandle = NULL;
TaskHandle_t handleEventFromQueueTaskHandler = NULL;
static const char* TAG = "MonocolorLED";
static uint8_t powerState = 1;

static void handleEventFromQueue(void* arg) {
    uint8_t inputGpioNumber;

    while(1) {
        if(xQueueReceive(*buttonQueueHandle, &inputGpioNumber, portMAX_DELAY)) {
            struct ChannelGpioMap* ptr = channelGpioMap;

            for (size_t i = 0; i < channelGpioMapSize; i++, ptr++)
            {
                if(ptr->inputGpioPin == inputGpioNumber) {
                    ESP_LOGI(TAG,"Changing state of channel %d to %d to target duty of %d \r", ptr->ledcChannel, !ptr->currentState, ptr->targetDuty);
                    ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, ptr->ledcChannel, ptr->currentState?0:ptr->targetDuty, 250);
                    // ledc_set_fade_time_and_start(LEDC_HIGH_SPEED_MODE, ptr->ledcChannel, ptr->currentState?0:ptr->targetDuty, 250, LEDC_FADE_NO_WAIT);
                    
                    ptr->currentState = !(ptr->currentState);
                    powerOn12vSource();

                    ledc_fade_start(LEDC_HIGH_SPEED_MODE, ptr->ledcChannel,LEDC_FADE_NO_WAIT);
                }
            }
            powerOff12vSource();
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

void powerOn12vSource() {
    gpio_num_t gpio = POWER_LED_12V_GPIO_PIN;

    struct ChannelGpioMap* ptr = channelGpioMap;
    for (size_t i = 0; i < channelGpioMapSize; i++, ptr++)
    {
        if(ptr->currentState) {

            if(!powerState) {
                printf("Power source is on, returning... \n\r");
                // Power source is on, returning
                return;    
            }

            // Power up source
            printf("Power up source... \n\r");
            powerState = 0;
            gpio_set_level(gpio, 0);
            vTaskDelay(750/portTICK_RATE_MS);
            return;
        } 
    }

    return;
}

void powerOff12vSource() {
    gpio_num_t gpio = POWER_LED_12V_GPIO_PIN;
    bool isAnyActive = false;

    struct ChannelGpioMap* ptr = channelGpioMap;
    for (size_t i = 0; i < channelGpioMapSize; i++, ptr++)
    {
        isAnyActive = isAnyActive | ptr -> currentState;
    }

    if(!isAnyActive) {
        vTaskDelay(750/portTICK_RATE_MS);
        // There are no active buttons, power off source.
        printf("No active inputs, power down... \n\r");
        powerState = 1;
        gpio_set_level(gpio, 1);
    }

    return;
}

void init12vPowerSource() {
    gpio_num_t gpio = POWER_LED_12V_GPIO_PIN;
    gpio_pad_select_gpio( (1ULL<<gpio));
    gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio, 1);
}