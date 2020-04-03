#include "buttons.h"

static xQueueHandle gpio_push_events_queue = NULL;
static xQueueHandle *button_queue_handle = NULL;
static const char *TAG = "Buttons";

static void IRAM_ATTR gpio_isr_handler(void *arg) {
  uint32_t gpio_num = (uint32_t)arg;
  gpio_isr_handler_remove(gpio_num);
  xQueueSendFromISR(gpio_push_events_queue, &gpio_num, NULL);
}

static void handleButtonPush(void *arg) {
  uint32_t io_num;
  for (;;) {
    if (xQueueReceive(gpio_push_events_queue, &io_num, portMAX_DELAY)) {
      vTaskDelay(80 / portTICK_RATE_MS);
      if (gpio_get_level(io_num) == 0) {
        ESP_LOGI(TAG, "Button GPIO[%d] pushed", io_num);
      }

      vTaskDelay(LONG_PRESS_DELAY / portTICK_RATE_MS);
      if (gpio_get_level(io_num) == 0) {
        ESP_LOGI(TAG, "Button GPIO[%d] pushed LONG", io_num);
      }

      xQueueSend(*button_queue_handle, &io_num, 0);
      gpio_isr_handler_add(io_num, gpio_isr_handler, (void *)io_num);
    }
  }
}

void init_buttons(xQueueHandle *queueHandler) {
  button_queue_handle = queueHandler;
  gpio_push_events_queue = xQueueCreate(10, sizeof(uint32_t));

  xTaskCreate(handleButtonPush, "buttonHandler", 2048, NULL, 10, NULL);
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
}

void add_button(uint8_t gpio_pin) {
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;
  io_conf.pin_bit_mask = GPIO_PIN_SEL_CALC(gpio_pin);
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en = 1;
  io_conf.pull_down_en = 0;
  gpio_config(&io_conf);

  gpio_isr_handler_add(gpio_pin, gpio_isr_handler, (void *)gpio_pin);
}