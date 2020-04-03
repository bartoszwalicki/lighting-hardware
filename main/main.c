#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#include "driver/gpio.h"
#include "nvs_flash.h"

#include "types.h"

#include "buttons.h"
#include "monocolor_led.h"
#include "mqtt_connection.h"
#include "wifi_connection.h"

QueueHandle_t button_actions_handle_queue = NULL;
QueueHandle_t mqtt_incoming_events_handle_queue = NULL;
struct ChannelGpioMap channel_gpio_map[SIZE_OF_GPIO_INPUTS] = {
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
  add_button(channel_gpio_map[0].input_gpio_pin);
  // Kitchen - sink
  add_channel(&channel_gpio_map[0]);
  // Kitchen - wine stand
  add_channel(&channel_gpio_map[1]);
}

void app_main(void) {
  button_actions_handle_queue = xQueueCreate(5, sizeof(uint8_t));
  mqtt_incoming_events_handle_queue =
      xQueueCreate(10, sizeof(struct MqttMessageEvent));

  init_buttons(&button_actions_handle_queue);
  init_leds(&button_actions_handle_queue);

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

  mqtt_init();
}
