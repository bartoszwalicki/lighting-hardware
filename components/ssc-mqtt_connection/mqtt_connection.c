#include "mqtt_connection.h"

const char *TAG = "MQTT";
esp_mqtt_client_handle_t _client = NULL;

void handle_mqtt_incoming_event(esp_mqtt_event_handle_t event) {
  char trunc_topic[20];
  char operation_mode;
  uint32_t value = 0;

  sprintf(trunc_topic, "%.*s", event->topic_len, event->topic);

  operation_mode = trunc_topic[event->topic_len - 1];

  trunc_topic[event->topic_len - 2] = 0;

  struct MqttMessageEvent message_to_queue;
  strcpy(message_to_queue.topic, trunc_topic);

  if (isdigit(*event->data) && operation_mode == 's') {
    sprintf(trunc_topic, "%.*s", event->data_len, event->data);

    sscanf(trunc_topic, "%d", &value);
  }

  message_to_queue.value = value;
  message_to_queue.operation = operation_mode;
  xQueueSend(mqtt_incoming_events_handle_queue, &message_to_queue, 0);
}

esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
  esp_mqtt_client_handle_t client = event->client;
  // your_context_t *context = event->context;
  switch (event->event_id) {
  case MQTT_EVENT_CONNECTED:
    // Set value
    esp_mqtt_client_subscribe(client, "kitchen/sink/s", 0);
    esp_mqtt_client_subscribe(client, "kitchen/wine/s", 0);
    esp_mqtt_client_subscribe(client, "bathroom/shower/s", 0);
    esp_mqtt_client_subscribe(client, "bathroom/mirror/s", 0);
    // Toggle with fade
    esp_mqtt_client_subscribe(client, "kitchen/sink/t", 0);
    esp_mqtt_client_subscribe(client, "kitchen/wine/t", 0);
    esp_mqtt_client_subscribe(client, "bathroom/shower/t", 0);
    esp_mqtt_client_subscribe(client, "bathroom/mirror/t", 0);
    // Get values
    esp_mqtt_client_subscribe(client, "kitchen/sink/g", 0);
    esp_mqtt_client_subscribe(client, "kitchen/wine/g", 0);
    esp_mqtt_client_subscribe(client, "bathroom/shower/g", 0);
    esp_mqtt_client_subscribe(client, "bathroom/mirror/g", 0);
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    handle_mqtt_incoming_event(event);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    break;
  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
  }
  return ESP_OK;
}

void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                        int32_t event_id, void *event_data) {
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base,
           event_id);
  mqtt_event_handler_cb(event_data);
}

void mqtt_init(void) {
  esp_mqtt_client_config_t mqtt_cfg = {
      .uri = CONFIG_BROKER_URL,
  };

  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  _client = client;
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 client);
  esp_mqtt_client_start(client);
}

void mqtt_publish(const char *topic, const char *data) {
  char valueTopic[20];
  sprintf(valueTopic, "%s/v", topic);

  esp_mqtt_client_publish(_client, valueTopic, data, 0, 0, 0);
}