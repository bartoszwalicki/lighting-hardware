#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "buttons.h"
#include "hello.h"



void app_main(void)
{
  initButtons();
  addButton(23);
  
  initHello();
}
