#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_timer.h"

#define SDA_PIN 21
#define SCL_PIN 22
#define ORNEK_SAYISI 100

static esp_err_t register_yaz(i2c_master_dev_handle_t cihaz,
                              uint8_t adres, uint8_t deger)
{
    uint8_t veri[2] = {adres, deger};
    return i2c_master_transmit(cihaz, veri, sizeof(veri), 100);
}

void app_main(void)
{
    i2c_master_bus_handle_t bus = NULL;

    i2c_master_bus_config_t bus_ayar = {
        .i2c_port = -1,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true};

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_ayar, &bus));

    printf("\nLAB 3 - BOLUM 2: I2C MPU6050\n");
    printf("SDA: GPIO21, SCL: GPIO22\n");
    printf("I2C adres taramasi basliyor...\n");

    int sensor_adresi = -1;
    int bulunan = 0;

    for (int adres = 0x08; adres < 0x78; adres++)
    {
        esp_err_t sonuc = i2c_master_probe(bus, adres, 50);

        if (sonuc == ESP_OK)
        {
            printf("Cihaz bulundu: 0x%02X\n", adres);
            bulunan++;

            if (adres == 0x68 || adres == 0x69)
            {
                sensor_adresi = adres;
            }
        }
        else if (sonuc != ESP_ERR_NOT_FOUND)
        {
            printf("Tarama hatasi: %s\n", esp_err_to_name(sonuc));
            printf("Baglantilari kontrol edin.\n");
            return;
        }
    }

    printf("Bulunan cihaz sayisi: %d\n", bulunan);

    if (sensor_adresi < 0)
    {
        printf("MPU6050 beklenen adreslerde bulunamadi.\n");
        return;
    }

    i2c_master_dev_handle_t sensor = NULL;

    i2c_device_config_t cihaz_ayar = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = sensor_adresi,
        .scl_speed_hz = 100000};

    ESP_ERROR_CHECK(
        i2c_master_bus_add_device(bus, &cihaz_ayar, &sensor));

    uint8_t kimlik_register = 0x75;
    uint8_t kimlik = 0;

    ESP_ERROR_CHECK(
        i2c_master_transmit_receive(
            sensor, &kimlik_register, 1, &kimlik, 1, 100));

    printf("WHO_AM_I: 0x%02X\n", kimlik);
    printf("Tekrarli kimlik kontrolu:\n");

    for (int i = 1; i <= 10; i++)
    {
        uint8_t tekrar_kimlik = 0;

        esp_err_t sonuc = i2c_master_transmit_receive(
            sensor, &kimlik_register, 1, &tekrar_kimlik, 1, 100);

        if (sonuc == ESP_OK)
        {
            printf("%d: WHO_AM_I = 0x%02X\n", i, tekrar_kimlik);
        }
        else
        {
            printf("%d: Okuma hatasi: %s\n",
                   i, esp_err_to_name(sonuc));
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }

    if (kimlik == 0x72)
    {
        printf("UYARI: Beklenen MPU6050 kimligi dogrulanamadi.\n");
        printf("0x72 kimlikli modulde register uyumluluk denemesi.\n");
    }
    else if (kimlik != 0x68)
    {
        printf("Desteklenmeyen kimlik. Test durduruldu.\n");
        return;
    }

    /* Uyku modundan cik; X gyro PLL saatini sec. */
    ESP_ERROR_CHECK(register_yaz(sensor, 0x6B, 0x01));
    vTaskDelay(pdMS_TO_TICKS(100));

    /* Ivme olcegi: +/-2 g, hassasiyet: 16384 LSB/g. */
    ESP_ERROR_CHECK(register_yaz(sensor, 0x1C, 0x00));

    /* Dijital alcak geciren filtre ayari. */
    ESP_ERROR_CHECK(register_yaz(sensor, 0x1A, 0x03));

    printf("Sensor hazir. Okuma hizi: 10 Hz\n");
    printf("Ilk 2 saniye sabit tutun, sonra hafifce egin.\n");
    printf("ornek,zaman_ms,ax_raw,ay_raw,az_raw\n");
    int64_t baslangic = esp_timer_get_time();
    TickType_t son_uyanma = xTaskGetTickCount();

    for (int i = 1; i <= ORNEK_SAYISI; i++)
    {
        uint8_t veri_register = 0x3B;
        uint8_t veri[6];

        esp_err_t sonuc = i2c_master_transmit_receive(
            sensor, &veri_register, 1, veri, sizeof(veri), 100);

        if (sonuc != ESP_OK)
        {
            printf("Okuma hatasi, ornek %d: %s\n",
                   i, esp_err_to_name(sonuc));
            return;
        }

        int16_t ax = (int16_t)(((uint16_t)veri[0] << 8) | veri[1]);
        int16_t ay = (int16_t)(((uint16_t)veri[2] << 8) | veri[3]);
        int16_t az = (int16_t)(((uint16_t)veri[4] << 8) | veri[5]);

        int64_t zaman_ms =
            (esp_timer_get_time() - baslangic) / 1000;

        printf("%d,%" PRId64 ",%d,%d,%d\n",
               i, zaman_ms, ax, ay, az);

        vTaskDelayUntil(&son_uyanma, pdMS_TO_TICKS(100));
    }

    printf("I2C testi tamamlandi. Okunan ornek: %d\n",
           ORNEK_SAYISI);
}