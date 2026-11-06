#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_err.h"

#define MOSI_PIN 23
#define MISO_PIN 19
#define SCLK_PIN 18
#define CS_PIN 5

static void mod_testi(int mod)
{
    spi_device_handle_t cihaz = NULL;

    spi_device_interface_config_t cihaz_ayar = {
        .clock_speed_hz = 1000000,
        .mode = mod,
        .spics_io_num = CS_PIN,
        .queue_size = 1};

    ESP_ERROR_CHECK(
        spi_bus_add_device(SPI2_HOST, &cihaz_ayar, &cihaz));

    printf("\n--- SPI MOD %d ---\n", mod);
    printf("CPOL: %d, CPHA: %d\n",
           (mod >> 1) & 1, mod & 1);
    printf("Saat ayari: 1 MHz\n");

    int basarili = 0;

    for (int i = 1; i <= 5; i++)
    {
        spi_transaction_t islem = {
            .flags = SPI_TRANS_USE_TXDATA |
                     SPI_TRANS_USE_RXDATA,
            .length = 32,
            .rxlength = 32,
            .tx_data = {0xA5, 0x3C, 0x0F, 0xF0}};

        ESP_ERROR_CHECK(
            spi_device_polling_transmit(cihaz, &islem));

        printf("\nTest %d\n", i);

        printf("Gonderilen:");
        for (int j = 0; j < 4; j++)
        {
            printf(" %02X", (unsigned int)islem.tx_data[j]);
        }

        printf("\nAlinan:");
        for (int j = 0; j < 4; j++)
        {
            printf(" %02X", (unsigned int)islem.rx_data[j]);
        }

        printf("\n");

        if (memcmp(islem.tx_data, islem.rx_data, 4) == 0)
        {
            printf("Sonuc: ESLESTI\n");
            basarili++;
        }
        else
        {
            printf("Sonuc: ESLESMEDI\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    printf("Mod %d eslesen test: %d/5\n", mod, basarili);

    ESP_ERROR_CHECK(spi_bus_remove_device(cihaz));
}

void app_main(void)
{
    spi_bus_config_t bus_ayar = {
        .mosi_io_num = MOSI_PIN,
        .miso_io_num = MISO_PIN,
        .sclk_io_num = SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4};

    ESP_ERROR_CHECK(
        spi_bus_initialize(
            SPI2_HOST, &bus_ayar, SPI_DMA_CH_AUTO));

    printf("\nLAB 3 - BOLUM 3: SPI GERI DONGU\n");
    printf("Baglanti: GPIO23 MOSI -> GPIO19 MISO\n");
    printf("SCLK: GPIO18, CS: GPIO5\n");
    printf("Her aktarim: 4 bayt, 32 bit\n");

    mod_testi(0);
    mod_testi(3);

    ESP_ERROR_CHECK(spi_bus_free(SPI2_HOST));

    printf("\nSPI geri dongu testi tamamlandi.\n");
}