
 #include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "string.h"

static const char *TAG = "SPI MASTER";

//GPIO para HSPI (SPI2)
#define GPIO_MOSI 23
#define GPIO_MISO 19
#define GPIO_SCLK 18
#define GPIO_CS   5

#define WHO_AM_I                    0x0F
#define OUT_X_H                     0x29
#define OUT_X_L                     0x28
#define OUT_Y_H                     0x2B
#define OUT_Y_L                     0x2A
#define OUT_Z_H                     0x2D
#define OUT_Z_L                     0x2C
#define CTRL1                       0x20

//HSPI
spi_device_handle_t spi2;

static esp_err_t spi_init() {
    esp_err_t ret;
    spi_bus_config_t buscfg={
        .miso_io_num = GPIO_MISO,
        .mosi_io_num = GPIO_MOSI,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
       // .max_transfer_sz = 32
    };
    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    spi_device_interface_config_t devcfg={
        .clock_speed_hz = 1.6,  
        .mode = 1,                  
        .spics_io_num = GPIO_CS,     
        .queue_size = 1,
        .pre_cb = NULL,
        //.post_cb = NULL,
        .address_bits = 8
    };
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi2);
    ESP_ERROR_CHECK(ret);
    return ret;
};

esp_err_t device_register_read(uint8_t reg_addr, uint8_t *data,  size_t len) {
    uint8_t tx_data = ((1<<7) | reg_addr);
    spi_transaction_t t = {
        .length = 8,
        .rx_buffer = data,
        .rxlength = len,
        .addr = tx_data
    };
    esp_err_t ret=spi_device_polling_transmit(spi2, &t);
    ESP_ERROR_CHECK(ret);
    return ret;
}

esp_err_t device_register_write(uint8_t reg_addr, uint8_t data) {
    uint8_t tx_data = ((1<<7) | reg_addr);
    spi_transaction_t t = {
        .addr = tx_data,
        .tx_buffer = &data,
        .length = 8
    };
    esp_err_t ret = spi_device_polling_transmit(spi2, &t);
    ESP_ERROR_CHECK(ret);
    return ret;
}

void app_main(void)
{
    uint8_t data;
    esp_err_t ret = spi_init();

    //CONFIGURAR MODO DE OPERACION DEL SENSOR (REG CTRL 1)
    data = 0; //00010101
    if ((ret = device_register_write(0x20, 0x15)) == ESP_OK){
        ESP_LOGI(TAG, "Sensor configurado correctamente");
        while (1)
        {
            if (device_register_read(WHO_AM_I,&data, 8) == ESP_OK){ printf("\nWHO_AM_I = 0x%x \n", data);}
            if (device_register_read(OUT_X_H, &data, 8) == ESP_OK){ printf("X high = 0x%x \n", data);}
            if (device_register_read(OUT_X_L, &data, 8) == ESP_OK){ printf("X low = 0x%x \n", data);}
            if (device_register_read(OUT_Y_H, &data, 8) == ESP_OK){ printf("Y high = 0x%x \n", data);}
            if (device_register_read(OUT_Y_L, &data, 8) == ESP_OK){ printf("Y low = 0x%x \n", data);}
            if (device_register_read(OUT_Z_H, &data, 8) == ESP_OK){ printf("Z high = 0x%x \n", data);}
            if (device_register_read(OUT_Z_L, &data, 8) == ESP_OK){ printf("Z low = 0x%x \n", data);} 

            vTaskDelay(3000 / portTICK_PERIOD_MS);
        }
    } else{
        ESP_LOGI(TAG, "Write error = %X", ret);
    } 

    //Never reached.
     //ret=spi_bus_remove_device(spi);
    //assert(ret==ESP_OK); 
}  