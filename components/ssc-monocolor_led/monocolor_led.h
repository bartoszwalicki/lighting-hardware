#ifndef MONOCOLOR_LED_H
#define MONOCOLOR_LED_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/ledc.h"
#include "esp_log.h"

#include "../../main/types.h"

#define DELAY_OF_POWER_OF_12V 10000

extern struct ChannelGpioMap channelGpioMap[];
extern const uint8_t channelGpioMapSize;

void powerOff12vSourceTask(void *pvParameters);
void init12vPowerSource();
void powerOn12vSource();
void powerOff12vSource();

uint8_t lookupLedcChannel(uint8_t* gpioPin);
void initLeds(xQueueHandle* queueHandler);
void addChannel(struct ChannelGpioMap* channelConfig);

#endif