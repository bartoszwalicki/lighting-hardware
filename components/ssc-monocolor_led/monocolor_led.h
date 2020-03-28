#ifndef MONOCOLOR_LED_H
#define MONOCOLOR_LED_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/ledc.h"
#include "esp_log.h"

#include "../../main/types.h"

#include "mqtt_connection.h"

#define DELAY_POWER_OFF_12V 10000

extern struct ChannelGpioMap channelGpioMap[];
extern QueueHandle_t mqttIncomingEventsHandleQueue;

bool is_any_on(uint8_t input_gpio_pin);
bool is_any_on_global(void);
void full_toggle_led_with_fade(uint8_t input_gpio_pin);
void powerOff12vSourceTask(void *pvParameters);
void setLedState(struct ChannelGpioMap* channelInfo, bool sendMqtt, int customDuty);
void init12vPowerSource();
void powerOn12vSource();
void schedulePowerOf12vSource();

uint8_t lookupLedcChannel(uint8_t* gpioPin);
void initLeds(xQueueHandle* queueHandler);
void addChannel(struct ChannelGpioMap* channelConfig);

#endif