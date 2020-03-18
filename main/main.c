#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "buttons.h"
#include "hello.h"
#include "monocolor_led.h"

QueueHandle_t buttonActionsHandleQueue = NULL;

void app_main(void)
{
  buttonActionsHandleQueue = xQueueCreate(5, sizeof(uint8_t));

  initButtons(&buttonActionsHandleQueue);
  addButton(23);

  initLeds(&buttonActionsHandleQueue);
  addChannel(5);

  //initHello();
}
