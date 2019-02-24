#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "driver/gpio.h"

#include "./uart.h"
#include "./gpio.h"

/*-----------------------------------------------------------
 * GPIO CONFIGURATION
 *----------------------------------------------------------*/

void init_pins()
{
    gpio_set_intr_type(GPIO_LIGHT_SENSOR, GPIO_INTR_ANYEDGE);
    gpio_set_direction(GPIO_LIGHT_SENSOR, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_LIGHT_SENSOR, GPIO_PULLDOWN_ONLY);

    // Trick ESP8266: We need to set some pin to out zero in order to use GPIO02 as input
    // https://www.forward.com.au/pfod/ESP8266/GPIOpins/index.html
    gpio_set_intr_type(_GPIO_LIGHT_SENSOR_OUT, GPIO_INTR_DISABLE);
    gpio_set_direction(_GPIO_LIGHT_SENSOR_OUT, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(_GPIO_LIGHT_SENSOR_OUT, GPIO_FLOATING);
    gpio_set_level(_GPIO_LIGHT_SENSOR_OUT, 0);
}
