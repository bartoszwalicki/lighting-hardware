#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "types.h"

#include "buttons.h"
#include "hello.h"
#include "monocolor_led.h"

QueueHandle_t buttonActionsHandleQueue = NULL;

void app_main(void)
{
  buttonActionsHandleQueue = xQueueCreate(5, sizeof(uint8_t));

  const uint8_t gpioMapSize = 1;
  struct ChannelGpioMap channelGpioMap[1] = {
    {.inputGpioPin = 23, .outputLedChannelPin = 5, .ledcChannel = LEDC_CHANNEL_0}
  };

  initButtons(&buttonActionsHandleQueue);
  initLeds(&buttonActionsHandleQueue, &channelGpioMap, &gpioMapSize);
  
  struct ChannelGpioMap* ptr = channelGpioMap;

  for(int i=0; i<gpioMapSize; i++, ptr++) {
    if(ptr->inputGpioPin && ptr->outputLedChannelPin) {
      addButton(ptr->inputGpioPin);
      addChannel(ptr->outputLedChannelPin);
    }
  }
}
