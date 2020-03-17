#ifndef BUTTON_H
#define BUTTON_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "driver/gpio.h"

#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_PIN_SEL_CALC(gpio) (1ULL<<gpio)
#define LONG_PRESS_DELAY 500

void initButtons(void);
void addButton(uint8_t gpioPin);

#endif