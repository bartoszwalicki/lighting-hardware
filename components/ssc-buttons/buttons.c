#include "buttons.h"

static xQueueHandle gpio_evt_queue = NULL;

static void IRAM_ATTR gpioIsrHandler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    gpio_isr_handler_remove(gpio_num);
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void handleButtonPush(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            vTaskDelay(80/portTICK_RATE_MS);
            if(gpio_get_level(io_num) == 0) {
                printf("GPIO[%d] intr, val: %d %d\n", io_num, gpio_get_level(io_num), rand() % 50);
            }

            vTaskDelay(LONG_PRESS_DELAY/portTICK_RATE_MS);
            if(gpio_get_level(io_num) == 0) {
                printf("GPIO[%d] intr, val: %d %d LONG \n", io_num, gpio_get_level(io_num), rand() % 50);
            }


            gpio_isr_handler_add(io_num, gpioIsrHandler, (void*) io_num);
        }
    }
}

void initButtons() {
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(handleButtonPush, "buttonHandler", 2048, NULL, 10, NULL);
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
}

void addButton(uint8_t gpioPin) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;
    io_conf.pin_bit_mask = GPIO_PIN_SEL_CALC(gpioPin);  
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    io_conf.pull_down_en = 0;
    gpio_config(&io_conf);

    gpio_isr_handler_add(gpioPin, gpioIsrHandler, (void*) gpioPin);
}