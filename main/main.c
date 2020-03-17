#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "buttons.h"
#include "hello.h"
#include "leds.h"

void setLogging(void) {
  esp_log_level_set("buttons", ESP_LOG_VERBOSE);
}

void app_main(void)
{
  setLogging();

  initButtons();
  addButton(23);

  initLeds();
  addChannel(5);

  initHello();
}
