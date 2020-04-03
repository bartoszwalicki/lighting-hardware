#ifndef MQTT_H
#define MQTT_H

#include "esp_log.h"
#include "mqtt_client.h"

#include "../../main/types.h"

extern QueueHandle_t mqtt_incoming_events_handle_queue;

void mqtt_init(void);
void mqtt_publish(const char *topic, const char *data);

#endif