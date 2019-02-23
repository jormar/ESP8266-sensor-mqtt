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
    gpio_set_intr_type(GPIO_OUTPUT_IO_0, GPIO_INTR_DISABLE);
    gpio_set_direction(GPIO_OUTPUT_IO_0, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(GPIO_INPUT_IO_0, GPIO_FLOATING);
    // gpio_set_level(GPIO_OUTPUT_IO_0, 0);

    // gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_NEGEDGE);
    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);
    gpio_set_direction(GPIO_INPUT_IO_0, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_INPUT_IO_0, GPIO_PULLUP_ONLY);
    // gpio_set_pull_mode(GPIO_INPUT_IO_0, GPIO_PULLDOWN_ONLY);
}
