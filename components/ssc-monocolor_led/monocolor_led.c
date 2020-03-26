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

            for (size_t i = 0; i < SIZE_OF_GPIO_INPUTS; i++, ptr++)
            {
                if(ptr->inputGpioPin == inputGpioNumber) {
                   setLedState(ptr, true, -1);
                }
            }
            schedulePowerOf12vSource();
        }

        vTaskDelay(50/ portTICK_RATE_MS);
    }
};

static void handleIncomingEventFromMqttQueue(void* arg) {
    uint8_t messageToQueue;

    while(1) {
        if(xQueueReceive(mqttIncomingEventsHandleQueue, &messageToQueue, portMAX_DELAY)) {
            printf("Received! %d\n\r", 10);
        }

        vTaskDelay(50/ portTICK_RATE_MS);
    }
};

void initLeds(xQueueHandle* queueHandler) {
    buttonQueueHandle = queueHandler;

    ledc_fade_func_install(0);

    xTaskCreate(handleEventFromQueue,"handleEventFromQueue", 2048, NULL, tskIDLE_PRIORITY, &handleEventFromQueueTaskHandler);
    xTaskCreate(handleIncomingEventFromMqttQueue, "mqttEventQueue", 2048, NULL, tskIDLE_PRIORITY, &handleEventFromQueueTaskHandler);
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
    for (size_t i = 0; i < SIZE_OF_GPIO_INPUTS; i++, ptr++)
    {
        if(ptr->currentState) {

            if(powerOffTaskHandler != NULL) {
                ESP_LOGD(TAG, "Deleting power off task\r");
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

void schedulePowerOf12vSource() {
    bool isAnyActive = false;

    struct ChannelGpioMap* ptr = channelGpioMap;
    for (size_t i = 0; i < SIZE_OF_GPIO_INPUTS; i++, ptr++)
    {
        isAnyActive = isAnyActive | ptr -> currentState;
    }

    if(!isAnyActive) {
        xTaskCreate(powerOff12vSourceTask, "powerOff12", 2048, NULL, tskIDLE_PRIORITY, &powerOffTaskHandler);
    }

    return;
}

void init12vPowerSource() {
    gpio_pad_select_gpio(POWER_LED_12V_GPIO_PIN);
    gpio_set_direction(POWER_LED_12V_GPIO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(POWER_LED_12V_GPIO_PIN, 1);
}

void powerOff12vSourceTask(void *pvParameters) {
    ESP_LOGD(TAG, "Power off planned\r");
    vTaskDelay(DELAY_POWER_OFF_12V/portTICK_RATE_MS);

    powerPinState = 1;
    gpio_set_level(POWER_LED_12V_GPIO_PIN, 1);

    ESP_LOGI(TAG, "12V source powered off\r");

    TaskHandle_t tempHandler = powerOffTaskHandler;
    powerOffTaskHandler = NULL;

    vTaskDelete(tempHandler);
}

void setLedState(struct ChannelGpioMap* channelInfo, bool sendMqtt, int customDuty) {
    uint32_t selectedDuty = -1;

    if(customDuty > -1) {
        selectedDuty = customDuty;
    } else {
        selectedDuty = channelInfo->currentState?0:channelInfo->targetDuty;
    }

    ESP_LOGI(TAG,"Changing state of channel %d to %d to target duty of %d \r", channelInfo->ledcChannel, !channelInfo->currentState, channelInfo->targetDuty);
    ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, channelInfo->ledcChannel, selectedDuty, 450);
   
    channelInfo->currentState = selectedDuty == 0? false: !(channelInfo->currentState);
    powerOn12vSource();

    ledc_fade_start(LEDC_HIGH_SPEED_MODE, channelInfo->ledcChannel,LEDC_FADE_NO_WAIT);

    if(sendMqtt) {
        char temp[5];
        sprintf(temp, "%d", selectedDuty);
        mqttPublish(channelInfo->topic, temp);
    }
}