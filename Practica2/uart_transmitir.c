
/*-------------TRANSMITIR DATOS-------------*/

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "mUart.h"

#define UART_RX_PIN     (3)
#define UART_TX_PIN     (1)

#define UART_RX_PIN_2    (16)
#define UART_TX_PIN_2    (17)

#define UARTS_BAUD_RATE         (115200)
#define TASK_STACK_SIZE         (1048)
#define READ_BUF_SIZE           (1024)

#define BUF_SIZE (1024)
#define LED_GPIO (2)

#define MAX 50

#define CABECERA 0x5A
#define FIN 0xB2

#define DEFAULT_COMMAND 0x30 // '0'
#define DEFAULT_LENGTH 0x34  // '4'
#define DEFAULT_DATA "0000"

struct paquete{
    uint8_t  cabecera;
    uint8_t  comando;
    uint8_t  longitud;
    char dato1;
    char dato2;
    char dato3;
    char dato4;
    uint8_t  fin;
    uint8_t CRC32_1;
    uint8_t CRC32_2;
    uint8_t CRC32_3;
    uint8_t CRC32_4;
};

struct paquete* formar_paquete(uint8_t cabecera, uint8_t com, uint8_t length, char data[], uint8_t end, uint32_t crc){
    struct paquete *p;
    p=(struct paquete*)malloc(sizeof (struct paquete));
    p->cabecera = cabecera;
    p->comando = com;
    p->longitud = length;
    p->dato1 = data[0];
    p->dato2 = data[1];
    p->dato3 = data[2];
    p->dato4 = data[3];
    p->fin = end;
    p->CRC32_1 =  crc & 0xff;
    p->CRC32_2 = (crc & 0xff00)>>8;
    p->CRC32_3 = (crc & 0xff0000)>>16;
    p->CRC32_4 = (crc & 0xff000000)>>24;
    return (p);
}

void UartInit(uart_port_t uart_num, uint32_t baudrate, uint8_t size, uint8_t parity, uint8_t stop, uint8_t txPin, uint8_t rxPin){
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

void DelayMs(uint16_t ms){
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void UartPutchar(uart_port_t uart_num, char c){
    uart_write_bytes(uart_num, &c, sizeof(c));
}

char UartGetchar(uart_port_t uart_num){
    char c;
    uart_read_bytes(uart_num, &c, sizeof(c), 0);
    return c;
} 

void UartPuts(uart_port_t uart_num, char *str){
    while(*str!='\0'){   
        UartPutchar(uart_num,*(str++));
    }
}

void UartGets(uart_port_t uart_num, char *str){
    uint8_t cad=50;
    char c;
    const char *in=str;
    int i = 0;

    c=UartGetchar(uart_num);
    while (i < 50) {
        if (c == '\n'){
            break;
        }
       if(str < (in + cad - 1)){
            *(str) = c;
            str++;
        }
        c=UartGetchar(uart_num);
        i++;
    }
    *str = 0;
    str -= i;
} 

uint32_t crc32b(char *message) {
   int i, j;
   unsigned int byte, crc, mask;
   i = 0;
   crc = 0xFFFFFFFF;
   while (message[i]) {
      byte = message[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
      i++;
   }
   return ~crc;
}

void preprocess_string_for_crc32(char *str, char *datos, uint8_t comando){
    *str = CABECERA;
    *(++str) = comando;
    *(++str) = strlen(datos) + '0';    
    while(*datos){
        *(++str) = *(datos++);
    }
    *(++str) = FIN;
    *(++str) = 0;
}

void myItoa(uint16_t number, char* str, uint8_t base)
{//convierte un valor numerico en una cadena de texto
    char *str_aux = str, n, *end_ptr, ch;
    int i=0, j=0;

    do{
        n=number % base;
        number/=base;
        n += '0';
        if(n >'9')
            n += 7;
        *(str++) = n;
        j++;
    }while(number>0);

    *(str--)='\0';
    end_ptr = str;
  
    for (i = 0; i < j / 2; i++) {
        ch = *end_ptr;
        *(end_ptr--) = *str_aux;
        *(str_aux++) = ch;
    }
}

uint16_t myAtoi(char *str){
    uint16_t res = 0;
    while(*str){
        if((*str >= 48) && (*str <= 57)){
            res *= 10;
            res += *str-48;
            str++;
        }
        else
            break;
    }
    return res;
}

uint8_t package_validation(char *str, char *datos){
    int contador=0, data_length=0;
    uint32_t crc_recibido = 0, crc_calculado;
    char crc32_aux[13]; //MAX
    if (str[0] != CABECERA){ 
        printf(" CABECERA %x", str[0]);
        return 0;
    }else{ //COMANDO
        if (str[1] != 0x30){ //este if se puede unir con el de arriba 
            printf(" COMANDO ");
            return 0;
        } else { //LONGITUD
            if (str[2] != '0') {
                data_length = str[2] - '0'; 
                //DATOS
                while (contador < data_length){
                    datos[contador] = str[contador + 3];
                    contador++;
                }
                datos[contador] = '\0';
                contador = 0;
            } else{
                data_length = 1;
            }
            if (str[data_length+3] != FIN){
                printf(" FIN ");
                return 0;
            }else{
                crc_recibido |= (str[data_length+4] | str[data_length+5] << 8 | str[data_length+6] << 16  | str[data_length+7] << 24);
                preprocess_string_for_crc32(crc32_aux, datos, str[1]);
                crc_calculado = crc32b(crc32_aux);
                if(crc_recibido!=crc_calculado){
                    printf(" CRC32 ");
                    return 0;
                }
            }
        }
    }
    return 1;
}

void app_main(void){

    uint8_t comando = 0x10;
    uint32_t crc32_calculado;
    char str_aux_for_crc32[13], paquete_recibido[MAX], datos_recibidos[5];
    int com = 1;

    struct paquete * package;

    UartInit(0, UARTS_BAUD_RATE, 8, 0, 1, UART_TX_PIN,   UART_RX_PIN);
    UartInit(2, UARTS_BAUD_RATE, 8, 0, 1, UART_TX_PIN_2, UART_RX_PIN_2);

    while(1) {
        //MANDAR
        switch (com){ 
            case 1: comando = 0x10;
                break;
            case 2: comando = 0x11;
                break;
            case 3: comando = 0x12;
                break;
            case 4: comando = 0x13;
                break;
            default: break;
        }

        preprocess_string_for_crc32(str_aux_for_crc32, DEFAULT_DATA, comando);
        crc32_calculado = crc32b(str_aux_for_crc32);
        package = formar_paquete(CABECERA, comando, DEFAULT_LENGTH, DEFAULT_DATA, FIN, crc32_calculado); 
        printf("\nComando: %x", comando);
        UartPuts(2, package); 
        
        //RECIBIR
        UartGets(2, paquete_recibido);
        if (package_validation(paquete_recibido, datos_recibidos)){
            printf("\nRespuesta: %s\n", datos_recibidos);
        } else {
            printf("\nPaquete malformado\n");
        }
        memset(paquete_recibido, 0, 50);
        com = com == 4 ? 1 : com + 1;

        DelayMs(5000);
    }
} 