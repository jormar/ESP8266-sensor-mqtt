#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "MQTTClient.h"
#include "MQTTFreeRTOS.h"

#include "./uart.h"
#include "./wifi.h"
#include "./mqtt.h"

static const char *TAG = "MQTT";

// static void defaultMessageHandler(MessageData *data)
// {
//     ESP_LOGI(TAG, "Message arrived[len:%u]: %.*s", data->message->payloadlen, data->message->payloadlen, (char *)data->message->payload);
// }

MQTTClient client;
Network network;
char clientID[256] = {0};
EventGroupHandle_t mqtt_event_group;
const int MQTT_CONNECTED_BIT = BIT0;
MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
TaskHandle_t mqttClientMonitorTaskHandler = NULL;

static bool initMqtt()
{
    NetworkInit(&network);
    return MQTTClientInit(&client, &network, 0, NULL, 0, NULL, 0);
}

static bool connectMqtt()
{
    int rc = 0;

    PRINTF_ON_UART("MQTT - Waiting wifi connect...\n");
    // Block until wifi is connected
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);

    if ((rc = NetworkConnect(&network, CONFIG_MQTT_BROKER, CONFIG_MQTT_PORT)) != 0) {
        ESP_LOGE(TAG, "Return code from network connect is %d", rc);
        return pdFALSE;
    }

    sprintf(clientID, "%s_%u", CONFIG_MQTT_CLIENT_ID, esp_random());
    connectData.MQTTVersion = CONFIG_DEFAULT_MQTT_VERSION;
    connectData.clientID.cstring = clientID;
    connectData.keepAliveInterval = CONFIG_MQTT_KEEP_ALIVE;
    connectData.cleansession = CONFIG_DEFAULT_MQTT_SESSION;
    if (strlen(CONFIG_MQTT_USERNAME))
    {
        connectData.username.cstring = CONFIG_MQTT_USERNAME;
        connectData.password.cstring = CONFIG_MQTT_PASSWORD;
    }

    PRINTF_ON_UART("MQTT Connecting...\n");

    if ((rc = MQTTConnect(&client, &connectData)) != 0) {
        ESP_LOGE(TAG, "Return code from MQTT connect is %d", rc);
        network.disconnect(&network);
        return pdFALSE;
    }

#if defined(MQTT_TASK)
    if ((rc = MQTTStartTask(&client)) != pdPASS) {
        ESP_LOGE(TAG, "Return code from start tasks is %d", rc);
    } else {
        PRINTF_ON_UART("Use MQTTStartTask\n");
    }
#endif

    PRINTF_ON_UART("MQTT Connected!\n");
    xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
    return pdTRUE;
}

static bool disconnectMqtt()
{
    int rc = 0;
    bool r = pdTRUE;

    if ((rc = MQTTDisconnect(&client)) != 0) {
        ESP_LOGE(TAG, "Return code from MQTT disconnect is %d", rc);
        r = pdFALSE;
    }
    network.disconnect(&network);
    xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
    return r;
}

static bool reconnectMqtt()
{
    disconnectMqtt();
    // network.disconnect(&network);
    return connectMqtt();
}

static void vMqttClientMonitorTask(void *arg)
{
    // TaskStatus_t pxTaskStatus;
    mqtt_event_group = xEventGroupCreate();
    if (initMqtt() == false)
    {
        ESP_LOGE(TAG, "mqtt init err");
        vTaskDelete(NULL);
    }

    connectMqtt();

    while(pdTRUE) {
        // PRINTF_ON_UART("MQTT loop\n");
        EventBits_t mqttBits = xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED_BIT, false, true, 0);
        EventBits_t wifiBits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, 0);
        if (!(mqttBits & MQTT_CONNECTED_BIT) || !(wifiBits & WIFI_CONNECTED_BIT)) {
            // MQTT Not connected
            PRINTF_ON_UART("MQTT will be reconnected...\n");
            reconnectMqtt();
        }

        // vTaskGetInfo(mqttClientMonitorTaskHandler, &pxTaskStatus, pdTRUE, eInvalid);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGW(TAG, "mqtt_client_thread going to be deleted");
    vTaskDelete(NULL);
    return;
}

int publishToMqttTopic(char* text, const char* topic, const int retained, const int qos)
{
    int rc = 0;
    MQTTMessage message;

    EventBits_t mqttBits = xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED_BIT, false, true, pdMS_TO_TICKS(3000));
    if (mqttBits & MQTT_CONNECTED_BIT) {
        message.qos = qos;
        message.retained = retained;
        message.payload = (void *)text;
        message.payloadlen = strlen(text);
        if ((rc = MQTTPublish(&client, topic, &message)) != 0) {
            PRINTF_ON_UART("Return code from MQTT publish is %d", rc);
            return rc;
            // reconnectMqtt();
        } else {
            PRINTF_ON_UART("MQTT published topic %s, len:%u heap:%u\n", topic, message.payloadlen, esp_get_free_heap_size());
            return 0;
        }
    } else {
        return -1;
    }
}

// void subscribeToMqttTopic(const char* topic, int qos, messageHandler messageHandler){}

void initializeMqttTasks()
{
    xTaskCreate(vMqttClientMonitorTask, "vMqttClientMonitorTask", 4096, NULL, 0, &mqttClientMonitorTaskHandler);
}
