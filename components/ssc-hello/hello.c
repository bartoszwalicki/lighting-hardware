#include "hello.h"

TaskHandle_t helloTaskHandler = NULL;

void sayHello(void *p) {
    int counter = 0;

    while(1) {
        printf("Hello! %i \n\r", counter++);
        vTaskDelay(1000/ portTICK_RATE_MS);
    }
}

void initHello(void) {
    xTaskCreate(sayHello, "sayHello", 2048, NULL, tskIDLE_PRIORITY, &helloTaskHandler);
}