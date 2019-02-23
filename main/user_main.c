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
#include "nvs_flash.h"

#include "../components/utils/uart.h"
#include "../components/utils/gpio.h"
#include "../components/utils/wifi.h"
#include "../components/utils/mqtt.h"

TaskHandle_t gpioSensorTaskHandler = NULL;
TaskHandle_t sendDataOfSensorsTaskHandler = NULL;

static void vSendDataOfSensorsTask(void *arg)
{
    static int lastValueSend = -1;
    int sensorValue;
    
    while (pdTRUE)
    {
        // light weight binary semaphore
        if(ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
        {
            sensorValue = gpio_get_level(GPIO_LIGHT_SENSOR);
            if (lastValueSend != sensorValue) {
                lastValueSend = sensorValue;
                printf_on_uart("sendDataOfSensors - %d\n", sensorValue);

                // TODO: Send status by MQTT
            } else {
                printf_on_uart("sendDataOfSensors (EQUAL)- %d\n", sensorValue);
            }
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
    TimerHandle_t *xTimer = xTimerCreate("Sensor Timer", pdMS_TO_TICKS(1000), pdFALSE, ( void * ) 0, vSensorTimerCallback);

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

    // printf_on_uart("SDK version:%s\n", esp_get_idf_version());

    xTaskCreate(vGpioSensorTask, "vGpioSensorTask", 4096, NULL, 1, &gpioSensorTaskHandler);
    xTaskCreate(vSendDataOfSensorsTask, "vSendDataOfSensorsTask", 4096, NULL, 1, &sendDataOfSensorsTaskHandler);

    gpio_install_isr_service(0); //install gpio isr service
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpioSensorHandlerISR, NULL);
    // gpio_isr_handler_add(GPIO_INPUT_IO_0, gpioSensorHandlerISR, (void *) GPIO_INPUT_IO_0);
}
