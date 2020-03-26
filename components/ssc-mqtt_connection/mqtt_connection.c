#include "mqtt_connection.h"

const char *TAG = "MQTT";
esp_mqtt_client_handle_t _client = NULL;

void handleMqttIncomingEvent(esp_mqtt_event_handle_t event) {
    char truncTopic[20];
    uint32_t value = 0;
    
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);

    sprintf(truncTopic, "%.*s", event->topic_len, event->topic); ;
    truncTopic[event->topic_len - 2] = 0;

    if(isdigit(*event->data)) {
        sscanf(event->data, "%d", &value);
    }

    struct MqttMessageEvent messageToQueue;
    memcpy(messageToQueue.topic, truncTopic, 20);
    messageToQueue.value = value;
    //strcpy(messageToQueue.topic,truncTopic);

    // fprintf("TRUNC: %s %d\n\r", messageToQueue.topic, messageToQueue.value);
    printf("Queue pointer: %p \n\r", &mqttIncomingEventsHandleQueue);
    uint8_t temp = 10;
    xQueueSend(mqttIncomingEventsHandleQueue, &temp, 0);
}

esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            esp_mqtt_client_subscribe(client, "kitchen/sink/s", 0);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            handleMqttIncomingEvent(event);
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

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void mqttInit(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    _client = client;
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void mqttPublish(const char * topic, const char * data) {
    char valueTopic[20];
    sprintf(valueTopic, "%s/v", topic);

    esp_mqtt_client_publish(_client, valueTopic, data, 0, 0, 0);
}