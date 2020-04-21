#include "dimm_storage.h"

nvs_handle_t nvs_handler;
esp_err_t err;

void save_dimm(uint8_t channel, uint32_t dimm_value) {
  nvs_open("storage", NVS_READWRITE, &nvs_handler);

  char buff[25];
  sprintf(buff, "ch_dimm_%i", channel);

  err = nvs_set_u32(nvs_handler, buff, dimm_value);
  err = nvs_commit(nvs_handler);
  if (err != ESP_OK) {
    ESP_LOGE("NVS", "Problem with saving dimm value to nvs");
  }

  nvs_close(nvs_handler);
}

uint32_t get_dimm(uint8_t channel) {
  nvs_open("storage", NVS_READWRITE, &nvs_handler);

  char buff[25];
  sprintf(buff, "ch_dimm_%i", channel);

  uint32_t dimm_value =
      1500; // value will default to 1500, if not set yet in NVS

  err = nvs_get_u32(nvs_handler, buff, &dimm_value);
  switch (err) {
  case ESP_ERR_NVS_NOT_FOUND:
    ESP_LOGI("NVS", "Value of channel %s not in NVS", buff);
    break;
  }

  nvs_close(nvs_handler);
  return dimm_value;
}