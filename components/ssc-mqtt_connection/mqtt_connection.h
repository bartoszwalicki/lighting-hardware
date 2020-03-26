#ifndef MQTT_H
#define MQTT_H

#include "mqtt_client.h"
#include "esp_log.h"

#include "../../main/types.h"

extern QueueHandle_t mqttIncomingEventsHandleQueue;

void mqttInit(void);
void mqttPublish(const char * topic, const char * data);

#endif