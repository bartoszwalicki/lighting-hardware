#include "monocolor_led.h"

static xQueueHandle *button_queue_handle = NULL;

TaskHandle_t handle_event_from_queue_task_handler = NULL;
TaskHandle_t handle_event_from_button_queue_task_handler = NULL;
TaskHandle_t power_off_task_handler = NULL;

static const char *TAG = "MonocolorLED";

static uint8_t power_pin_state = 0;

void get_current_duty(struct ChannelGpioMap *channel_info) {
  char temp[5];
  sprintf(temp, "%d", channel_info->current_duty);
  mqtt_publish(channel_info->topic, temp);
}

static void handle_event_from_button_queue(void *arg) {
  struct ButtonEvent button_event;

  while (1) {
    if (xQueueReceive(*button_queue_handle, &button_event, portMAX_DELAY)) {
      if (button_event.action_type == 0) {
        full_toggle_led_with_fade(button_event.input_gpio_pin, false);
      }
      if (button_event.action_type == 1) {
        full_toggle_led_with_fade(button_event.input_gpio_pin, true);
      }
    }

    vTaskDelay(50 / portTICK_RATE_MS);
  }
};

static void handle_incoming_event_from_mqtt_queue(void *arg) {
  struct MqttMessageEvent message_to_queue;

  while (1) {
    if (xQueueReceive(mqtt_incoming_events_handle_queue, &message_to_queue,
                      portMAX_DELAY)) {

      struct ChannelGpioMap *ptr = channel_gpio_map;
      for (size_t i = 0; i < SIZE_OF_GPIO_INPUTS; i++, ptr++) {
        if (!strcmp(ptr->topic, message_to_queue.topic)) {
          switch (message_to_queue.operation) {
          case 's':
            set_led_state(ptr, true, message_to_queue.value);
            break;
          case 't':
            full_toggle_led_with_fade(ptr->input_gpio_pin, false);
            break;
          case 'g':
            get_current_duty(ptr);
            break;
          case 'u':
            power_on_with_fade(ptr->input_gpio_pin);
            break;
          case 'd':
            power_off_with_fade(ptr->input_gpio_pin);
            break;
          case 'x':
            save_half_dimm_value(ptr, message_to_queue.value);
            break;
          default:
            break;
          }
        }
      }
    }

    vTaskDelay(50 / portTICK_RATE_MS);
  }
};

void init_leds(xQueueHandle *button_queue_handler) {
  button_queue_handle = button_queue_handler;

  ledc_fade_func_install(0);

  read_half_dimm_values();

  xTaskCreate(handle_event_from_button_queue, "handleEventFromQueue", 4096,
              NULL, tskIDLE_PRIORITY,
              &handle_event_from_button_queue_task_handler);
  xTaskCreate(handle_incoming_event_from_mqtt_queue, "mqttEventQueue", 2048,
              NULL, tskIDLE_PRIORITY, &handle_event_from_queue_task_handler);
}

void add_channel(struct ChannelGpioMap *channelConfig) {
  ledc_timer_config_t ledc_timer = {.duty_resolution = LEDC_TIMER_12_BIT,
                                    .freq_hz = 16000,
                                    .speed_mode = LEDC_HIGH_SPEED_MODE,
                                    .timer_num = LEDC_TIMER_0};

  ledc_timer_config(&ledc_timer);

  ledc_channel_config_t ledc_channel = {
      .channel = channelConfig->led_channel,
      .duty = 0,
      .gpio_num = channelConfig->output_led_channel_pin,
      .speed_mode = LEDC_HIGH_SPEED_MODE,
      .timer_sel = LEDC_TIMER_0};

  ledc_channel_config(&ledc_channel);
}

void power_on_12v_source() {
  struct ChannelGpioMap *ptr = channel_gpio_map;
  for (size_t i = 0; i < SIZE_OF_GPIO_INPUTS; i++, ptr++) {
    if (ptr->current_state) {

      if (power_off_task_handler != NULL) {
        ESP_LOGD(TAG, "Deleting power off task\r");
        vTaskDelete(power_off_task_handler);
        power_off_task_handler = NULL;
      }

      if (power_pin_state) {
        ESP_LOGD(TAG, "12V power source is on, returning\r");
        return;
      }

      ESP_LOGI(TAG, "Powering up 12V source\r");
      power_pin_state = 1;
      gpio_set_level(POWER_LED_12V_GPIO_PIN, 1);

      // Wait until hardware power source will spin up
      vTaskDelay(750 / portTICK_RATE_MS);
      return;
    }
  }

  return;
}

void schedule_power_off_12v_source() {
  bool is_any_active = is_any_on_global();

  if (!is_any_active) {
    xTaskCreate(power_off_12v_source_task, "powerOff12", 2048, NULL,
                tskIDLE_PRIORITY, &power_off_task_handler);
  }

  return;
}

void init_12v_power_source() {
  gpio_pad_select_gpio(POWER_LED_12V_GPIO_PIN);
  gpio_set_direction(POWER_LED_12V_GPIO_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(POWER_LED_12V_GPIO_PIN, 0);
}

void power_off_12v_source_task(void *pvParameters) {
  ESP_LOGI(TAG, "Power off planned\r");
  vTaskDelay(DELAY_POWER_OFF_12V / portTICK_RATE_MS);

  power_pin_state = 0;
  gpio_set_level(POWER_LED_12V_GPIO_PIN, 0);

  ESP_LOGI(TAG, "12V source powered off\r");

  TaskHandle_t temp_handler = power_off_task_handler;
  power_off_task_handler = NULL;

  vTaskDelete(temp_handler);
}

void set_led_state(struct ChannelGpioMap *channel_info, bool send_mqtt,
                   int custom_duty) {
  uint32_t selected_duty = -1;

  if (custom_duty > -1) {
    selected_duty = custom_duty;
  } else {
    selected_duty = channel_info->current_state ? 0 : channel_info->target_duty;
  }

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel_info->led_channel, selected_duty);

  channel_info->current_state = selected_duty == 0 ? false : true;
  channel_info->current_duty = selected_duty;

  power_on_12v_source();

  ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel_info->led_channel);

  ESP_LOGI(TAG, "Changing state of channel %d to %d to target duty of %d \r",
           channel_info->led_channel, !channel_info->current_state,
           channel_info->target_duty);

  if (send_mqtt) {
    char temp[5];
    sprintf(temp, "%d", selected_duty);
    mqtt_publish(channel_info->topic, temp);
  }

  if (selected_duty == 0) {
    schedule_power_off_12v_source();
  }
}

void power_on_with_fade(uint8_t input_gpio_pin) {
  struct ChannelGpioMap *ptr = channel_gpio_map;
  for (size_t i = 0; i < SIZE_OF_GPIO_INPUTS; i++, ptr++) {
    if (input_gpio_pin == ptr->input_gpio_pin) {
      uint32_t target_duty = ptr->target_duty;
      ptr->current_duty = target_duty;
      ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, ptr->led_channel,
                              target_duty, FADE_TIME);

      ptr->current_state = target_duty == 0 ? false : true;
      power_on_12v_source();

      ESP_LOGI(TAG,
               "Changing state of channel %d to %d to target duty of %d \r",
               ptr->led_channel, ptr->current_state, target_duty);
      ledc_fade_start(LEDC_HIGH_SPEED_MODE, ptr->led_channel,
                      LEDC_FADE_NO_WAIT);

      char temp[5];
      sprintf(temp, "%d", target_duty);
      mqtt_publish(ptr->topic, temp);
    }
  }
  schedule_power_off_12v_source();
}

void power_off_with_fade(uint8_t input_gpio_pin) {
  struct ChannelGpioMap *ptr = channel_gpio_map;
  for (size_t i = 0; i < SIZE_OF_GPIO_INPUTS; i++, ptr++) {
    if (input_gpio_pin == ptr->input_gpio_pin) {
      uint32_t target_duty = 0;
      ptr->current_duty = target_duty;
      ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, ptr->led_channel,
                              target_duty, FADE_TIME);

      ptr->current_state = target_duty == 0 ? false : true;
      power_on_12v_source();

      ESP_LOGI(TAG,
               "Changing state of channel %d to %d to target duty of %d \r",
               ptr->led_channel, ptr->current_state, target_duty);
      ledc_fade_start(LEDC_HIGH_SPEED_MODE, ptr->led_channel,
                      LEDC_FADE_NO_WAIT);

      char temp[5];
      sprintf(temp, "%d", target_duty);
      mqtt_publish(ptr->topic, temp);
    }
  }
  schedule_power_off_12v_source();
}

void full_toggle_led_with_fade(uint8_t input_gpio_pin, bool half_fade) {
  bool is_any_on_state = is_any_on(input_gpio_pin);

  struct ChannelGpioMap *ptr = channel_gpio_map;
  for (size_t i = 0; i < SIZE_OF_GPIO_INPUTS; i++, ptr++) {
    if (input_gpio_pin == ptr->input_gpio_pin) {
      uint32_t target_duty = is_any_on_state == 0 ? ptr->target_duty : 0;

      if (target_duty > 0 && half_fade) {
        target_duty = ptr->half_dimm;
      }

      ptr->current_duty = target_duty;
      ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, ptr->led_channel,
                              target_duty, FADE_TIME);

      ptr->current_state = target_duty == 0 ? false : true;
      power_on_12v_source();

      ESP_LOGI(TAG,
               "Changing state of channel %d to %d to target duty of %d \r",
               ptr->led_channel, ptr->current_state, target_duty);
      ledc_fade_start(LEDC_HIGH_SPEED_MODE, ptr->led_channel,
                      LEDC_FADE_NO_WAIT);

      char temp[5];
      sprintf(temp, "%d", target_duty);
      mqtt_publish(ptr->topic, temp);
    }
  }
  schedule_power_off_12v_source();
}

bool is_any_on(uint8_t input_gpio_pin) {
  bool is_any_active = false;

  struct ChannelGpioMap *ptr = channel_gpio_map;
  for (size_t i = 0; i < SIZE_OF_GPIO_INPUTS; i++, ptr++) {
    if (input_gpio_pin == ptr->input_gpio_pin) {
      is_any_active = is_any_active | ptr->current_state;
    }
  }

  return is_any_active;
}

bool is_any_on_global(void) {
  bool is_any_active = false;

  struct ChannelGpioMap *ptr = channel_gpio_map;
  for (size_t i = 0; i < SIZE_OF_GPIO_INPUTS; i++, ptr++) {
    is_any_active = is_any_active | ptr->current_state;
  }

  return is_any_active;
}

void read_half_dimm_values() {
  ESP_LOGI(TAG, "Entering reading procedure");

  struct ChannelGpioMap *ptr = channel_gpio_map;
  for (size_t i = 0; i < SIZE_OF_GPIO_INPUTS; i++, ptr++) {
    ptr->half_dimm = get_dimm(ptr->output_led_channel_pin);
    ESP_LOGI(TAG, "Setting half-dim for %s to %i", ptr->topic, ptr->half_dimm);
  }
}

void save_half_dimm_value(struct ChannelGpioMap *channel_info,
                          uint32_t dimm_value) {
  ESP_LOGI(TAG, "Setting half-dimm value of topic %s to %i",
           channel_info->topic, dimm_value);
  channel_info->half_dimm = dimm_value;
  save_dimm(channel_info->output_led_channel_pin, dimm_value);
  char temp[5];
  sprintf(temp, "%d", dimm_value);
  mqtt_publish_channel_half_dimm(channel_info->topic, temp);
}