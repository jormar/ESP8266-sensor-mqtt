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

static void vMqttClientTask(void *arg)
{
    MQTTClient client;
    Network network;
    int rc = 0;
    char clientID[64] = {0};
    uint32_t count = 0;

    // printf_on_uart("ssid:%s passwd:%s sub:%s qos:%u\n",
    //         CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD, CONFIG_MQTT_PUB_TOPIC,
    //         CONFIG_DEFAULT_MQTT_PUB_QOS);

    // printf_on_uart("ver:%u clientID:%s keepalive:%d username:%s passwd:%s session:%d level:%u",
    //          CONFIG_DEFAULT_MQTT_VERSION, CONFIG_MQTT_CLIENT_ID,
    //          CONFIG_MQTT_KEEP_ALIVE, CONFIG_MQTT_USERNAME, CONFIG_MQTT_PASSWORD,
    //          CONFIG_DEFAULT_MQTT_SESSION, CONFIG_DEFAULT_MQTT_SECURITY);

    // printf_on_uart("broker:%s port:%u", CONFIG_MQTT_BROKER, CONFIG_MQTT_PORT);

    // printf_on_uart("sendbuf:%u recvbuf:%u sendcycle:%u recvcycle:%u",
    //          CONFIG_MQTT_SEND_BUFFER, CONFIG_MQTT_RECV_BUFFER,
    //          CONFIG_MQTT_SEND_CYCLE, CONFIG_MQTT_RECV_CYCLE);

    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    NetworkInit(&network);
    if (MQTTClientInit(&client, &network, 0, NULL, 0, NULL, 0) == false) {
        ESP_LOGE(TAG, "mqtt init err");
        vTaskDelete(NULL);
    }

    // payload = malloc(CONFIG_MQTT_PAYLOAD_BUFFER);

    // if (!payload) {
    //     ESP_LOGE(TAG, "mqtt malloc err");
    // } else {
    //     memset(payload, 0x0, CONFIG_MQTT_PAYLOAD_BUFFER);
    // }

    while(pdTRUE) {
        printf_on_uart("MQTT - Waiting wifi connect...\n");
        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);

        if ((rc = NetworkConnect(&network, CONFIG_MQTT_BROKER, CONFIG_MQTT_PORT)) != 0) {
            ESP_LOGE(TAG, "Return code from network connect is %d", rc);
            continue;
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
        printf_on_uart("MQTT Connecting\n");
        if ((rc = MQTTConnect(&client, &connectData)) != 0) {
            ESP_LOGE(TAG, "Return code from MQTT connect is %d", rc);
            network.disconnect(&network);
            continue;
        }
        printf_on_uart("MQTT Connected!\n");

#if defined(MQTT_TASK)
        if ((rc = MQTTStartTask(&client)) != pdPASS) {
            ESP_LOGE(TAG, "Return code from start tasks is %d", rc);
        } else {
            printf_on_uart("Use MQTTStartTask\n");
        }
#endif

        // if ((rc = MQTTSubscribe(&client, CONFIG_MQTT_SUB_TOPIC, CONFIG_DEFAULT_MQTT_SUB_QOS, messageArrived)) != 0) {
        //     ESP_LOGE(TAG, "Return code from MQTT subscribe is %d", rc);
        //     network.disconnect(&network);
        //     continue;
        // }

        // printf_on_uart("MQTT subscribe to topic %s OK", CONFIG_MQTT_SUB_TOPIC);

        while(pdTRUE) {
            MQTTMessage message;
            char payload[32] = "";

            message.qos = CONFIG_DEFAULT_MQTT_PUB_QOS;
            message.retained = 1;
            message.payload = payload;
            snprintf(payload, 31, "%d", ++count % 2);
            message.payloadlen = strlen(payload);

            if ((rc = MQTTPublish(&client, CONFIG_MQTT_PUB_TOPIC, &message)) != 0) {
                ESP_LOGE(TAG, "Return code from MQTT publish is %d", rc);
            } else {
                printf_on_uart("MQTT published topic %s, len:%u heap:%u\n", CONFIG_MQTT_PUB_TOPIC, message.payloadlen, esp_get_free_heap_size());
            }

            if (rc != 0) {
                break;
            }

            vTaskDelay(pdMS_TO_TICKS(4000));
        }

        vTaskDelay(pdMS_TO_TICKS(30000));

        network.disconnect(&network);
    }

    ESP_LOGW(TAG, "mqtt_client_thread going to be deleted");
    vTaskDelete(NULL);
    return;
}

void sendMessageToMqttTopic(const char* message, const char* topic, int retained, int qos)
{

}

void initializeMqttTasks() {
    xTaskCreate(vMqttClientTask, "vMqttClientTask", 4096, NULL, 5, NULL);
}
