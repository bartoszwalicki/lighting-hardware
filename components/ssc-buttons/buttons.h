#ifndef BUTTON_H
#define BUTTON_H

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_PIN_SEL_CALC(gpio) (1ULL << gpio)
#define LONG_PRESS_DELAY 500

void initButtons(xQueueHandle *queueHandler);
void addButton(uint8_t gpioPin);

#endif