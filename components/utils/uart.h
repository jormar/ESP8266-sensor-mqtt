
void init_uart();

// void print_on_uart(const char *text);
int printf_on_uart(const char *format, ...);

#ifdef CONFIG_ALLOW_PRINTF_ON_UART
#define PRINTF_ON_UART( format, ... ) printf_on_uart(format, ##__VA_ARGS__)
#else
#define PRINTF_ON_UART( format, ... )
#endif
