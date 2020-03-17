#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "buttons.h"
#include "hello.h"
#include "leds.h"

QueueHandle_t buttonActionsHandleQueue = NULL;

void setLogging(void) {
  esp_log_level_set("buttons", ESP_LOG_VERBOSE);
}

void app_main(void)
{
  buttonActionsHandleQueue = xQueueCreate(5, sizeof(uint8_t));

  setLogging();

  initButtons(&buttonActionsHandleQueue);
  addButton(23);

  initLeds(&buttonActionsHandleQueue);
  addChannel(5);

  initHello();
}
