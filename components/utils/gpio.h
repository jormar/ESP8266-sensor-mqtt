
#include "driver/gpio.h"

#define GPIO_OUTPUT_IO_0    GPIO_NUM_0
#define GPIO_INPUT_IO_0     GPIO_NUM_2

#define _GPIO_LIGHT_SENSOR_OUT  GPIO_OUTPUT_IO_0 // We need this to use GPIO_NUM_2 as input pin
#define GPIO_LIGHT_SENSOR       GPIO_INPUT_IO_0

void init_pins();