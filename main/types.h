#ifndef TYPES_H
#define TYPES_H

struct ChannelGpioMap {
  uint8_t input_gpio_pin;
  uint8_t output_led_channel_pin;
  uint8_t led_channel;
  bool current_state;
  uint32_t target_duty;
  char topic[20];
};

struct MqttMessageEvent {
  char topic[20];
  uint32_t value;
};

#define SIZE_OF_GPIO_INPUTS 2
#define POWER_LED_12V_GPIO_PIN 19

#endif