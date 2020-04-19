#ifndef TYPES_H
#define TYPES_H

struct ChannelGpioMap {
  uint8_t input_gpio_pin;
  uint8_t output_led_channel_pin;
  uint8_t led_channel;
  bool current_state;
  uint32_t target_duty;
  uint32_t current_duty;
  char topic[20];
};

struct MqttMessageEvent {
  // MQTT operation codes
  // /s - set value (without fade)
  // /t - toggle with fade
  // /g - get current duty
  // /u - power on with fade
  // /d - power down with fade
  char topic[20];
  char operation;
  uint32_t value;
};

#define SIZE_OF_GPIO_INPUTS 4
#define POWER_LED_12V_GPIO_PIN 19

#endif