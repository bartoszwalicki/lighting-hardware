#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "types.h"

#include "buttons.h"
#include "hello.h"
#include "monocolor_led.h"

QueueHandle_t buttonActionsHandleQueue = NULL;
struct ChannelGpioMap channelGpioMap[SIZE_OF_GPIO_INPUTS] = {
  // Kitchen - sink
  {.inputGpioPin = 23, .outputLedChannelPin = 5, .ledcChannel = LEDC_CHANNEL_0, .currentState = false, .targetDuty = 4095},
  // Kitches - wine stand
  {.inputGpioPin = 23, .outputLedChannelPin = 4, .ledcChannel = LEDC_CHANNEL_1, .currentState = false, .targetDuty = 2000}
};

void configButtonsAndLeds() {
  // [Kitchen]
  addButton(channelGpioMap[0].inputGpioPin);
  // Kitchen - sink
  addChannel(&channelGpioMap[0]);
  // Kitchen - wine stand
  addChannel(&channelGpioMap[1]);
}

void app_main(void)
{
  buttonActionsHandleQueue = xQueueCreate(5, sizeof(uint8_t));

  initButtons(&buttonActionsHandleQueue);
  initLeds(&buttonActionsHandleQueue);

  init12vPowerSource();

  configButtonsAndLeds();
}
