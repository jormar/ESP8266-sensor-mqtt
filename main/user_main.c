/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/adc.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "../components/utils/uart.h"
#include "../components/utils/gpio.h"
#include "../components/utils/wifi.h"
#include "../components/utils/mqtt.h"

TaskHandle_t gpioSensorTaskHandler = NULL;
TaskHandle_t sendDataOfSensorsTaskHandler = NULL;

static void vSendDataOfSensorsTask(void *arg)
{
    char message[3] = {0};
    int sensorValue;
    
    while (pdTRUE)
    {
        // light weight binary semaphore
        if(ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
        {
            sensorValue = gpio_get_level(GPIO_LIGHT_SENSOR);
            sprintf(message, "%d", sensorValue);
            publishToMqttTopic(message, CONFIG_MQTT_PUB_TOPIC, 1, CONFIG_DEFAULT_MQTT_PUB_QOS);
            // esp_wifi_disconnect();
        }
    }
}

static void vSensorTimerCallback(TimerHandle_t xTimer)
{
    // Timer Service Task Context.
    // It is therefore essential that timer callback never attempt to block
    xTaskNotifyGive(sendDataOfSensorsTaskHandler);
}

static void vGpioSensorTask(void *arg)
{
    TimerHandle_t xTimer = xTimerCreate("SensorBufferTimer", pdMS_TO_TICKS(200), pdFALSE, ( void * ) 0, vSensorTimerCallback);

    while(pdTRUE)
    {
        // light weight binary semaphore
        if(ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
        {
            xTimerReset(xTimer, 0); // Let's wait until value get stable
        }
    }
}

static void gpioSensorHandlerISR(void *arg)
{
    // Interrupt Service Routine (ISR) Context
    vTaskNotifyGiveFromISR(gpioSensorTaskHandler,  NULL);
}

/******************************************************************************
 * FunctionName : app_main
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void app_main(void)
{
    init_uart();
    init_pins();
    initialise_wifi();
    initializeMqttTasks();

    // PRINTF_ON_UART("SDK version:%s\n", esp_get_idf_version());

    xTaskCreate(vGpioSensorTask, "vGpioSensorTask", 4096, NULL, 1, &gpioSensorTaskHandler);
    xTaskCreate(vSendDataOfSensorsTask, "vSendDataOfSensorsTask", 4096, NULL, 1, &sendDataOfSensorsTaskHandler);

    // Send current value every interval
    TimerHandle_t xTimer = xTimerCreate("SensorIntervalTimer", pdMS_TO_TICKS(10000), pdTRUE, ( void * ) 0, vSensorTimerCallback);
    xTimerStart(xTimer, 0);

    gpio_install_isr_service(0); //install gpio isr service
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpioSensorHandlerISR, NULL);
    // gpio_isr_handler_add(GPIO_INPUT_IO_0, gpioSensorHandlerISR, (void *) GPIO_INPUT_IO_0);
}
