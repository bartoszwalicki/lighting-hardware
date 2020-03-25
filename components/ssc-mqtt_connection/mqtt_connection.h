#ifndef MQTT_H
#define MQTT_H

#include "mqtt_client.h"
#include "esp_log.h"

void mqtt_app_start(void);
void mqttPublish(const char * topic, const char * data);

#endif