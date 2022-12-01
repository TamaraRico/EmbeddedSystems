/* UART Echo Example - GOOD ONE

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
#include "mUart.h"

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

typedef struct paquete
{
    uint8_t cabecera;
    uint8_t comando;
    uint8_t longitud;
    uint32_t datos;
    uint8_t fin;
    uint32_t CRC32;
};


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
    UartPutchar(uart_num, '\n');
}

void UartGets(uart_port_t uart_num, char *str)
{
    char c;
    int i=0;
    c=UartGetchar(uart_num);
    while (i<4) {/
        *(str++) = c;
        c=UartGetchar(uart_num);
        i++;
    }
    *str=0;
    str=str-i;
} 

void myItoa(uint16_t number, char* str, uint8_t base)
{//convierte un valor numerico en una cadena de texto
    char *str_aux = str, n, *end_ptr, ch;
    int i=0, j=0;

    do{
        n=number % base;
        number=number/base;
        n+='0';
        if(n>'9')
            n=n+7;
        *(str++)=n;
        j++;
    }while(number>0);

    *(str--)='\0';
    
    end_ptr = str;
  
    for (i = 0; i < j / 2; i++) {
        ch = *end_ptr;
        *end_ptr = *str_aux;
        *str_aux = ch;
          str_aux++;
        end_ptr--;
    }
}

#define TIMESTAMP_LEN 10
void enviar_timestamp(void){
    char str[TIMESTAMP_LEN];
    sprintf(str, "%d\n", xTaskGetTickCount());
    UartPuts(2, str);
}

void enviar_estado_led(void){
    gpio_set_direction(LED_GPIO, GPIO_MODE_INPUT_OUTPUT);
    int led = gpio_get_level(LED_GPIO);
    char led_state = led+'0';
    UartPutchar(2, led_state);
    UartPutchar(2, '\n');
}

void enviar_temperatura(void){
    int num = rand() % 100;
    char cad[4];
    myItoa(num, cad, 10);
    UartPuts(2, cad);
}

void invertir_estado_led(uint8_t led_state){
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, led_state);
    UartPutchar(2, '\n');
} 

void app_main(void)
{
    char str[5];
    
    UartInit(0, UARTS_BAUD_RATE, 8, 0, 1, UART_TX_PIN,   UART_RX_PIN);
    UartInit(2, UARTS_BAUD_RATE, 8, 0, 1, UART_TX_PIN_2, UART_RX_PIN_2);
    uint8_t led_state=1;
    while(1) 
    {
        UartGets(2, str);
        printf("gets: %s\n", str);
        if(!strcmp(str, "0x10")){
            enviar_timestamp();
        }else if(!strcmp(str, "0x11")){
            enviar_estado_led();
        }else if(!strcmp(str, "0x12")){
            enviar_temperatura();
        }else if(!strcmp(str, "0x13")){
            invertir_estado_led(led_state);
            led_state=!led_state;
        }   
        DelayMs(5000);
    }
}