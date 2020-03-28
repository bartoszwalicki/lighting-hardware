#ifndef TYPES_H
#define TYPES_H

struct ChannelGpioMap {
  uint8_t inputGpioPin;
  uint8_t outputLedChannelPin;
  uint8_t ledcChannel;
  bool currentState;
  uint32_t targetDuty;
  char topic[20];
};

struct MqttMessageEvent {
  char topic[20];
  uint32_t value;
};

#define SIZE_OF_GPIO_INPUTS 2
#define POWER_LED_12V_GPIO_PIN 19

#endif