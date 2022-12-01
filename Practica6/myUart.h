#ifndef MY_UART_H
#define MY_UART_H

#include "driver/uart.h"

void uartInit(uart_port_t uart_num, uint32_t baudrate, uint8_t size, uint8_t parity, uint8_t stop, uint8_t txPin, uint8_t rxPin);

// Send
void UartPuts(uart_port_t uart_num, char *str);
void UartPutchar(uart_port_t uart_num, char data);

// Receive
char UartGetchar(uart_port_t uart_num );
void UartGets(uart_port_t uart_num, char *str);


#endif