#ifndef DIMM_STORAGE_H
#define DIMM_STORAGE_H

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

void save_dimm(uint8_t channel, uint32_t dimm_value);
uint32_t get_dimm(uint8_t channel);

#endif