#ifndef MONOCOLOR_LED_H
#define MONOCOLOR_LED_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/ledc.h"
#include "esp_log.h"

#include "../../main/types.h"

void lookupLedcChannel(uint8_t* gpioPin);
void initLeds(xQueueHandle* queueHandler, struct ChannelGpioMap** map, const uint8_t* mapSize);
void addChannel(struct ChannelGpioMap* channelConfig);

#endif