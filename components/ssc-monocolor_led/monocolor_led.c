#include "monocolor_led.h"

static xQueueHandle* buttonQueueHandle = NULL;

TaskHandle_t handleEventFromQueueTaskHandler = NULL;
TaskHandle_t powerOffTaskHandler = NULL;

static const char* TAG = "MonocolorLED";

static uint8_t powerPinState = 1;



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
    struct ChannelGpioMap* ptr = channelGpioMap;
    for (size_t i = 0; i < channelGpioMapSize; i++, ptr++)
    {
        if(ptr->currentState) {

            if(powerOffTaskHandler != NULL) {
                printf("Delaying task!\n\r");
                vTaskDelete(powerOffTaskHandler);
                powerOffTaskHandler = NULL;
            }

            if(!powerPinState) {
                ESP_LOGD(TAG, "12V power source is on, returning\r");
                return;    
            }

            ESP_LOGI(TAG, "Powering up 12V source\r");
            powerPinState = 0;
            gpio_set_level(POWER_LED_12V_GPIO_PIN, 0);

            // Wait until hardware power source will spin up
            vTaskDelay(750/portTICK_RATE_MS);
            return;
        } 
    }

    return;
}

void powerOff12vSource() {
    bool isAnyActive = false;

    struct ChannelGpioMap* ptr = channelGpioMap;
    for (size_t i = 0; i < channelGpioMapSize; i++, ptr++)
    {
        isAnyActive = isAnyActive | ptr -> currentState;
    }

    if(!isAnyActive) {
        xTaskCreate(powerOff12vSourceTask, "powerOff12", 2048, NULL, tskIDLE_PRIORITY, &powerOffTaskHandler);
    }

    return;
}

void init12vPowerSource() {
    gpio_pad_select_gpio( (1ULL<<POWER_LED_12V_GPIO_PIN));
    gpio_set_direction(POWER_LED_12V_GPIO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(POWER_LED_12V_GPIO_PIN, 1);
}

void powerOff12vSourceTask(void *pvParameters) {
    printf("Power off planned\n\r");
    vTaskDelay(DELAY_OF_POWER_OF_12V/portTICK_RATE_MS);

    printf("No active inputs, power down... \n\r");
    powerPinState = 1;
    gpio_set_level(POWER_LED_12V_GPIO_PIN, 1);

    printf("Powered off...\n\r");

    TaskHandle_t tempHandler = powerOffTaskHandler;
    powerOffTaskHandler = NULL;

    vTaskDelete(tempHandler);
}