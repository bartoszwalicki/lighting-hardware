#ifndef MONOCOLOR_LED_H
#define MONOCOLOR_LED_H

#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdio.h>

#include "../../main/types.h"

#include "dimm_storage.h"
#include "mqtt_connection.h"

#define DELAY_POWER_OFF_12V 10000
#define FADE_TIME 1000

extern struct ChannelGpioMap channel_gpio_map[];
extern QueueHandle_t mqtt_incoming_events_handle_queue;

void read_half_dimm_values();
void save_half_dimm_value(struct ChannelGpioMap *channel_info,
                          uint32_t dimm_value);

bool is_any_on(uint8_t input_gpio_pin);
bool is_any_on_global(void);
void power_on_with_fade(uint8_t input_gpio_pin);
void power_off_with_fade(uint8_t input_gpio_pin);
void full_toggle_led_with_fade(uint8_t input_gpio_pin, bool half_fade);
void power_off_12v_source_task(void *pvParameters);
void set_led_state(struct ChannelGpioMap *channelInfo, bool sendMqtt,
                   int customDuty);
void init_12v_power_source();
void power_on_12v_source();
void schedule_power_off_12v_source();

uint8_t lookupled_channel(uint8_t *gpioPin);
void init_leds(xQueueHandle *queueHandler);
void add_channel(struct ChannelGpioMap *channelConfig);

#endif