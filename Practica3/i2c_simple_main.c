//TRANSMISOR

#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"

static const char *TAG = "i2c-simple-example";

#define I2C_MASTER_SCL_IO          22      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO          21      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM             0       /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define DEVICE_ADDR                 0x18        /*!< Address of the slave device */
                                    //Se tiene que conectar el pin SA0 del sensor a GND
#define WHO_AM_I                    0x0F
#define OUT_X_H                     0x29
#define OUT_X_L                     0x28
#define OUT_Y_H                     0x2B
#define OUT_Y_L                     0x2A
#define OUT_Z_H                     0x2D
#define OUT_Z_L                     0x2C
#define CTRL1                       0x20

#define ACK 0

/**
 * @brief Read a sequence of bytes from a device
 */
static esp_err_t device_read(uint8_t address, uint8_t *data, size_t len)
{
    return i2c_master_read_from_device(I2C_MASTER_NUM, address, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
}

/**
 * @brief Write a sequence of bytes to a device
 */
static esp_err_t device_write(uint8_t address, uint8_t *data, size_t len)
{
    return i2c_master_write_to_device(I2C_MASTER_NUM, address, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
}

/**
 * @brief Lee un registro del sensor LIS2DW12
 */
static esp_err_t device_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DEVICE_ADDR << 1), ACK);
    i2c_master_write_byte(cmd, reg_addr, ACK);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DEVICE_ADDR << 1) | 1, ACK);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

/**
* @brief Escribe un byte a un registro del sensor LIS2DW12
*/
esp_err_t device_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DEVICE_ADDR << 1), ACK);
    i2c_master_write_byte(cmd, reg_addr, ACK);
    i2c_master_write_byte(cmd, data, ACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}


/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}


void app_main(void)
{
    uint8_t data;
    esp_err_t ret;
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    //CONFIGURAR MODO DE OPERACION DEL SENSOR (REG CTRL 1)
    data = 0x15; //00010101
    if ((ret = device_register_write_byte(CTRL1, data)) == ESP_OK)
    {
        ESP_LOGI(TAG, "Sensor configurado correctamente");
        while (1)
        {
            if (device_register_read(WHO_AM_I, &data, 1) == ESP_OK){ printf("WHO_AM_I = 0x%x \n", data);}
            if (device_register_read(OUT_X_H, &data, 1) == ESP_OK){ printf("X high = 0x%x \n", data);}
            if (device_register_read(OUT_X_L, &data, 1) == ESP_OK){ printf("X low = 0x%x \n", data);}
            if (device_register_read(OUT_Y_H, &data, 1) == ESP_OK){ printf("Y high = 0x%x \n", data);}
            if (device_register_read(OUT_Y_L, &data, 1) == ESP_OK){ printf("Y low = 0x%x \n", data);}
            if (device_register_read(OUT_Z_H, &data, 1) == ESP_OK){ printf("Z high = 0x%x \n", data);}
            if (device_register_read(OUT_Z_L, &data, 1) == ESP_OK){ printf("Z low = 0x%x \n", data);}

            vTaskDelay(3000 / portTICK_PERIOD_MS);
        }
    }
    else
    {
        ESP_LOGI(TAG, "Write error = %X", ret);
    }


    ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
    ESP_LOGI(TAG, "I2C unitialized successfully");
}
