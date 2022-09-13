#ifndef M_UART_H
#define M_UART_H

#include "driver/uart.h"

/**
 * @brief Configure and install the default UART, then, connect it to the
 * console UART.
 */
void UartInit(uart_port_t uart_num, uint32_t baudrate, uint8_t size, uint8_t parity, uint8_t stop, uint8_t txPin, uint8_t rxPin);

/**
 * @brief Delay milliseconds
 */
void DelayMs(uint16_t ms);

/**
 * @brief Prints a character
 */
void UartPutchar(uart_port_t uart_num, char c);

/**
 * @brief Prints a string
 */
void UartPuts(uart_port_t uart_num, char *str);


#endif /* M_UART_H */