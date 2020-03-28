#include "monocolor_led.h"

static xQueueHandle *button_queue_handle = NULL;

TaskHandle_t handleEventFromQueueTaskHandler = NULL;
TaskHandle_t powerOffTaskHandler = NULL;

static const char *TAG = "MonocolorLED";

static uint8_t powerPinState = 1;

static void handle_event_from_button_queue(void *arg) {
  uint8_t input_gpio_number;

  while (1) {
    if (xQueueReceive(*button_queue_handle, &input_gpio_number,
                      portMAX_DELAY)) {
      full_toggle_led_with_fade(input_gpio_number);
    }

    vTaskDelay(50 / portTICK_RATE_MS);
  }
};

static void handle_incoming_event_from_mqtt_queue(void *arg) {
  struct MqttMessageEvent message_to_queue;

  while (1) {
    if (xQueueReceive(mqttIncomingEventsHandleQueue, &message_to_queue,
                      portMAX_DELAY)) {

      printf("Received! %s %d\n\r", message_to_queue.topic,
             message_to_queue.value);

      struct ChannelGpioMap *ptr = channelGpioMap;
      for (size_t i = 0; i < SIZE_OF_GPIO_INPUTS; i++, ptr++) {
        if (!strcmp(ptr->topic, message_to_queue.topic)) {
          setLedState(ptr, true, message_to_queue.value);
        }
      }
    }

    vTaskDelay(50 / portTICK_RATE_MS);
  }
};

void initLeds(xQueueHandle *button_queue_handler) {
  button_queue_handle = button_queue_handler;

  ledc_fade_func_install(0);

  xTaskCreate(handle_event_from_button_queue, "handleEventFromQueue", 2048,
              NULL, tskIDLE_PRIORITY, &handleEventFromQueueTaskHandler);
  xTaskCreate(handle_incoming_event_from_mqtt_queue, "mqttEventQueue", 2048,
              NULL, tskIDLE_PRIORITY, &handleEventFromQueueTaskHandler);
}

void addChannel(struct ChannelGpioMap *channelConfig) {
  ledc_timer_config_t ledc_timer = {.duty_resolution = LEDC_TIMER_12_BIT,
                                    .freq_hz = 16000,
                                    .speed_mode = LEDC_HIGH_SPEED_MODE,
                                    .timer_num = LEDC_TIMER_0};

  ledc_timer_config(&ledc_timer);

  ledc_channel_config_t ledc_channel = {.channel = channelConfig->ledcChannel,
                                        .duty = 0,
                                        .gpio_num =
                                            channelConfig->outputLedChannelPin,
                                        .speed_mode = LEDC_HIGH_SPEED_MODE,
                                        .timer_sel = LEDC_TIMER_0};

  ledc_channel_config(&ledc_channel);
}

void powerOn12vSource() {
  struct ChannelGpioMap *ptr = channelGpioMap;
  for (size_t i = 0; i < SIZE_OF_GPIO_INPUTS; i++, ptr++) {
    if (ptr->currentState) {

      if (powerOffTaskHandler != NULL) {
        ESP_LOGD(TAG, "Deleting power off task\r");
        vTaskDelete(powerOffTaskHandler);
        powerOffTaskHandler = NULL;
      }

      if (!powerPinState) {
        ESP_LOGD(TAG, "12V power source is on, returning\r");
        return;
      }

      ESP_LOGI(TAG, "Powering up 12V source\r");
      powerPinState = 0;
      gpio_set_level(POWER_LED_12V_GPIO_PIN, 0);

      // Wait until hardware power source will spin up
      vTaskDelay(750 / portTICK_RATE_MS);
      return;
    }
  }

  return;
}

void schedulePowerOf12vSource() {
  bool isAnyActive = is_any_on_global();

  if (!isAnyActive) {
    xTaskCreate(powerOff12vSourceTask, "powerOff12", 2048, NULL,
                tskIDLE_PRIORITY, &powerOffTaskHandler);
  }

  return;
}

void init12vPowerSource() {
  gpio_pad_select_gpio(POWER_LED_12V_GPIO_PIN);
  gpio_set_direction(POWER_LED_12V_GPIO_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(POWER_LED_12V_GPIO_PIN, 1);
}

void powerOff12vSourceTask(void *pvParameters) {
  ESP_LOGI(TAG, "Power off planned\r");
  vTaskDelay(DELAY_POWER_OFF_12V / portTICK_RATE_MS);

  powerPinState = 1;
  gpio_set_level(POWER_LED_12V_GPIO_PIN, 1);

  ESP_LOGI(TAG, "12V source powered off\r");

  TaskHandle_t tempHandler = powerOffTaskHandler;
  powerOffTaskHandler = NULL;

  vTaskDelete(tempHandler);
}

void setLedState(struct ChannelGpioMap *channelInfo, bool sendMqtt,
                 int customDuty) {
  uint32_t selectedDuty = -1;

  if (customDuty > -1) {
    selectedDuty = customDuty;
  } else {
    selectedDuty = channelInfo->currentState ? 0 : channelInfo->targetDuty;
  }

  ESP_LOGI(TAG, "Changing state of channel %d to %d to target duty of %d \r",
           channelInfo->ledcChannel, !channelInfo->currentState,
           channelInfo->targetDuty);
  ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, channelInfo->ledcChannel,
                          selectedDuty, 450);

  channelInfo->currentState =
      selectedDuty == 0 ? false : !(channelInfo->currentState);
  powerOn12vSource();

  ledc_fade_start(LEDC_HIGH_SPEED_MODE, channelInfo->ledcChannel,
                  LEDC_FADE_NO_WAIT);

  if (sendMqtt) {
    char temp[5];
    sprintf(temp, "%d", selectedDuty);
    mqttPublish(channelInfo->topic, temp);
  }

  if (selectedDuty == 0) {
    schedulePowerOf12vSource();
  }
}

void full_toggle_led_with_fade(uint8_t input_gpio_pin) {
  bool is_any_on_state = is_any_on(input_gpio_pin);

  struct ChannelGpioMap *ptr = channelGpioMap;
  for (size_t i = 0; i < SIZE_OF_GPIO_INPUTS; i++, ptr++) {
    if (input_gpio_pin == ptr->inputGpioPin) {
      uint32_t target_duty = is_any_on_state == 0 ? ptr->targetDuty : 0;
      ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, ptr->ledcChannel,
                              target_duty, 450);

      ptr->currentState = target_duty == 0 ? false : true;
      powerOn12vSource();

      ESP_LOGI(TAG,
               "Changing state of channel %d to %d to target duty of %d \r",
               ptr->ledcChannel, ptr->currentState, target_duty);
      ledc_fade_start(LEDC_HIGH_SPEED_MODE, ptr->ledcChannel,
                      LEDC_FADE_NO_WAIT);

      char temp[5];
      sprintf(temp, "%d", target_duty);
      mqttPublish(ptr->topic, temp);
    }
  }
  schedulePowerOf12vSource();
}

bool is_any_on(uint8_t input_gpio_pin) {
  bool is_any_active = false;

  struct ChannelGpioMap *ptr = channelGpioMap;
  for (size_t i = 0; i < SIZE_OF_GPIO_INPUTS; i++, ptr++) {
    if (input_gpio_pin == ptr->inputGpioPin) {
      is_any_active = is_any_active | ptr->currentState;
    }
  }

  return is_any_active;
}

bool is_any_on_global(void) {
  bool is_any_active = false;

  struct ChannelGpioMap *ptr = channelGpioMap;
  for (size_t i = 0; i < SIZE_OF_GPIO_INPUTS; i++, ptr++) {
    is_any_active = is_any_active | ptr->currentState;
  }

  return is_any_active;
}