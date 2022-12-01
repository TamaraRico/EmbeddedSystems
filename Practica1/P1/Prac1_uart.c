/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "myUart.h"


#define ECHO_TEST_TXD (CONFIG_EXAMPLE_UART_TXD)
#define ECHO_TEST_RXD (CONFIG_EXAMPLE_UART_RXD)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM      (CONFIG_EXAMPLE_UART_PORT_NUM)
#define ECHO_UART_BAUD_RATE     (CONFIG_EXAMPLE_UART_BAUD_RATE)
#define ECHO_TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)

#define UART_RX_PIN     (3)
#define UART_TX_PIN     (1)

#define UART_RX_PIN_2    (16)
#define UART_TX_PIN_2    (17)

#define UARTS_BAUD_RATE         (115200)
#define TASK_STACK_SIZE         (1048)
#define READ_BUF_SIZE           (1024)

#define BUF_SIZE (1024)
#define LED_GPIO (2)

void UartInit(uart_port_t uart_num, uint32_t baudrate, uint8_t size, uint8_t parity, uint8_t stop, uint8_t txPin, uint8_t rxPin)
{
    uart_config_t uart_config = {
        .baud_rate = (int) baudrate,
        .data_bits = (uart_word_length_t)(size-5),
        .parity    = (uart_parity_t)parity,
        .stop_bits = (uart_stop_bits_t)stop,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    ESP_ERROR_CHECK(uart_driver_install(uart_num, READ_BUF_SIZE, READ_BUF_SIZE, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(uart_num, txPin, rxPin,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

}

void DelayMs(uint16_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void UartPutchar(uart_port_t uart_num, char c)
{
    uart_write_bytes(uart_num, &c, sizeof(c));
}

char UartGetchar(uart_port_t uart_num){
    char c;
    uart_read_bytes(uart_num, &c, sizeof(c), 0);
    return c;
} 

void UartPuts(uart_port_t uart_num, char *str)
{
    while(*str!='\0')
    {   
        UartPutchar(uart_num,*str);
        (str++);
    }
    //UartPutchar(uart_num, '!');
}

void UartGets(uart_port_t uart_num, char *str)
{
    uint8_t cad=50;
    char c;
    const char *in=str;
    int i = 0;

    c=UartGetchar(uart_num);
    while (i <50) {
        if (c == '\n')
        {
            break;
        }
        
       if(str<(in + cad-1)){
            *str = c;
            str++;
        }
        c=UartGetchar(uart_num);
        i++;
    }
    *str=0;
    str = str-i;
} 
#define NUM_OF_COMMANDS   4
const char commands[NUM_OF_COMMANDS][5] =
{
    "0x10",
    "0x11",
    "0x12",
    "0x13"
};

#define STR_BUF_LEN 50

void app_main(void)
{
    char str[]="0x10";
    char str2[50];
    char str3[50];
    int com = 0;

    UartInit(0, UARTS_BAUD_RATE, 8, 0, 1, UART_TX_PIN,   UART_RX_PIN);
    UartInit(2, UARTS_BAUD_RATE, 8, 0, 1, UART_TX_PIN_2, UART_RX_PIN_2);

    while(1) 
    {
        switch (com)
        {
        case 1:
            str[3]='0';
            break;
        case 2:
            str[3]='1';
            break;

        case 3:
            str[3]='2';
            break;

        case 4:
            str[3]='3';
            break;
        
        default:
            break;
        }
        UartPuts(2,str);

        UartGets(2,str2);
        
        //Mostrar texto que llego
        UartPuts(0,"Respuesta recibida: ");
        UartPuts(0,str2);
        UartPuts(0,"\n");

        memset(str2, 0, 50);

        if (com == 4)
        {
            com = 1;
        }
        else{
            com++;
        }  
        DelayMs(5000);
    }
}
