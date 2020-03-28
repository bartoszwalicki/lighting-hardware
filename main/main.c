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
    {.input_gpio_pin = 23,
     .output_led_channel_pin = 5,
     .led_channel = LEDC_CHANNEL_0,
     .current_state = false,
     .target_duty = 4095,
     .topic = "kitchen/sink\0"},
    // Kitches - wine stand
    {.input_gpio_pin = 23,
     .output_led_channel_pin = 4,
     .led_channel = LEDC_CHANNEL_1,
     .current_state = false,
     .target_duty = 2000,
     .topic = "kitchen/wine\0"}};

void config_buttons_and_leds() {
  // [Kitchen]
  addButton(channelGpioMap[0].input_gpio_pin);
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

  config_buttons_and_leds();

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
