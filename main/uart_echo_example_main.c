/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

/**
 * This is an example which echos any data it receives on configured UART back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: configured UART
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below (See Kconfig)
 */

#define ECHO_TEST_TXD (CONFIG_EXAMPLE_UART_TXD)
#define ECHO_TEST_RXD (CONFIG_EXAMPLE_UART_RXD)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM      (CONFIG_EXAMPLE_UART_PORT_NUM)
#define ECHO_UART_BAUD_RATE     (CONFIG_EXAMPLE_UART_BAUD_RATE)
#define ECHO_TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)

#define BUF_SIZE (1024)

#define LED_GPIO (2)

static void echo_task(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, BUF_SIZE, 20 / portTICK_RATE_MS);
        // Write data back to the UART
        uart_write_bytes(ECHO_UART_PORT_NUM, (const char *) data, len);
        if(len){
            data[len] = '\0';
            ESP_LOGI("TAG", "Recv str: %s", (char*)data);
        }
    }
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

void delayMs(uint16_t ms)
{
    vTaskDelay(ms/portTICK_PERIOD_MS);
}

void enviar_timestamp(void){
    time_t seconds = time(NULL);
}

void enviar_estado_led(void){
     uart_write_bytes(ECHO_UART_PORT_NUM, (const char *) gpio_get_level(LED_GPIO), 1); //check for string length , number 
}

void enviar_temperatura(void){
    int r = rand() % 20;
    char cad[20];
    myItoa(num, cad, 10);
    uart_write_bytes(ECHO_UART_PORT_NUM, (const char *) r, 1); //check for string length , number 
}

void invertir_estado_led(void){
    gpio_set_level(LED_GPIO, !gpio_get_level(LED_GPIO));
    delayMs(10);
}

void app_main(void)
{
    //xTaskCreate(echo_task, "uart_echo_task", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
    while(1){
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        printf("led low");
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("led high");
    }
}
