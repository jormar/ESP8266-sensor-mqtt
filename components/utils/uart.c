#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"

#include "driver/uart.h"
#include "driver/gpio.h"

#include "esp_system.h"

/*-----------------------------------------------------------
 * UART CONFIGURATION
 *----------------------------------------------------------*/

#define BUF_SIZE (1024)

void init_uart()
{
#ifdef CONFIG_ALLOW_PRINTF_ON_UART
    // Configure parameters of an UART driver,
    // communication pins and install the driver
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL);
#else
    PIN_FUNC_SELECT(PERIPHS_GPIO_MUX_REG(GPIO_NUM_1), FUNC_GPIO1);
    PIN_FUNC_SELECT(PERIPHS_GPIO_MUX_REG(GPIO_NUM_3), FUNC_GPIO3);
#endif
}

void print_on_uart(const char *text)
{
    uart_write_bytes(UART_NUM_0, text, strlen(text));
}

int printf_on_uart(const char *format, ...)
{
    static char buffer[1024] = {0};
    va_list args;
    va_start(args, format);
    int sSize = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    return uart_write_bytes(UART_NUM_0, buffer, sSize);
}