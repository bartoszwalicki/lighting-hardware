#ifndef TYPES_H
#define TYPES_H

struct ChannelGpioMap {
  uint8_t input_gpio_pin;
  uint8_t output_led_channel_pin;
  uint8_t led_channel;
  bool current_state;
  uint32_t target_duty;
  uint32_t current_duty;
  uint32_t half_dimm;
  char topic[20];
};

struct ButtonEvent {
  // Button action types
  // 0 - short press
  // 1 - long press
  uint8_t input_gpio_pin;
  uint8_t action_type;
};

struct MqttMessageEvent {
  // MQTT operation codes
  // /s - set value (without fade)
  // /t - toggle with fade
  // /g - get current duty
  // /v - current duty value
  // /u - power on with fade
  // /d - power down with fade
  // /x - set half-fade value
  // /z - current half-fade value
  char topic[20];
  char operation;
  uint32_t value;
};

#define SIZE_OF_GPIO_INPUTS 4
#define POWER_LED_12V_GPIO_PIN 19

#endif