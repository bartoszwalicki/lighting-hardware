#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#include "driver/gpio.h"
#include "nvs_flash.h"

#include "types.h"

#include "buttons.h"
#include "hello.h"
#include "monocolor_led.h"
#include "mqtt_connection.h"
#include "wifi_connection.h"

QueueHandle_t buttonActionsHandleQueue = NULL;
QueueHandle_t mqttIncomingEventsHandleQueue = NULL;
struct ChannelGpioMap channelGpioMap[SIZE_OF_GPIO_INPUTS] = {
    // Kitchen - sink
    {.inputGpioPin = 23,
     .outputLedChannelPin = 5,
     .ledcChannel = LEDC_CHANNEL_0,
     .currentState = false,
     .targetDuty = 4095,
     .topic = "kitchen/sink\0"},
    // Kitches - wine stand
    {.inputGpioPin = 23,
     .outputLedChannelPin = 4,
     .ledcChannel = LEDC_CHANNEL_1,
     .currentState = false,
     .targetDuty = 2000,
     .topic = "kitchen/wine\0"}};

void configButtonsAndLeds() {
  // [Kitchen]
  addButton(channelGpioMap[0].inputGpioPin);
  // Kitchen - sink
  add_channel(&channelGpioMap[0]);
  // Kitchen - wine stand
  add_channel(&channelGpioMap[1]);
}

void app_main(void) {
  buttonActionsHandleQueue = xQueueCreate(5, sizeof(uint8_t));
  mqttIncomingEventsHandleQueue =
      xQueueCreate(10, sizeof(struct MqttMessageEvent));

  initButtons(&buttonActionsHandleQueue);
  init_leds(&buttonActionsHandleQueue);

  init_12v_power_source();

  configButtonsAndLeds();

  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  wifi_init_sta();

  mqttInit();
}
