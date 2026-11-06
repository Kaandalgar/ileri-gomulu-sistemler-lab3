#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_err.h"

#define UART_PORT UART_NUM_1
#define TX_PIN 17
#define RX_PIN 18

void app_main(void)
{
    uart_config_t ayar = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT};

    ESP_ERROR_CHECK(
        uart_driver_install(UART_PORT, 1024, 0, 0, NULL, 0));

    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &ayar));

    ESP_ERROR_CHECK(
        uart_set_pin(
            UART_PORT,
            TX_PIN,
            RX_PIN,
            UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE));

    printf("\nLAB 3 - BOLUM 1: UART GERI DONGU\n");
    printf("TX: GPIO17, RX: GPIO18\n");
    printf("Ayar: 115200 baud, 8N1\n");

    int basarili = 0;

    for (int i = 1; i <= 5; i++)
    {
        char gonderilen[64];
        uint8_t alinan[64] = {0};

        snprintf(gonderilen, sizeof(gonderilen),
                 "Merhaba ESP32 - test %d", i);

        int uzunluk = (int)strlen(gonderilen);

        ESP_ERROR_CHECK(uart_flush_input(UART_PORT));

        int yazilan = uart_write_bytes(
            UART_PORT, gonderilen, uzunluk);

        ESP_ERROR_CHECK(
            uart_wait_tx_done(UART_PORT, pdMS_TO_TICKS(1000)));

        int okunan = uart_read_bytes(
            UART_PORT,
            alinan,
            sizeof(alinan) - 1,
            pdMS_TO_TICKS(200));

        printf("\nTest %d\n", i);
        printf("Gonderilen: %s\n", gonderilen);

        if (okunan > 0)
        {
            alinan[okunan] = '\0';
            printf("Alinan: %s\n", (char *)alinan);
        }
        else
        {
            printf("Alinan: veri yok veya okuma hatasi\n");
        }

        if (yazilan == uzunluk &&
            okunan == uzunluk &&
            memcmp(gonderilen, alinan, uzunluk) == 0)
        {

            printf("Sonuc: BASARILI\n");
            basarili++;
        }
        else
        {
            printf("Sonuc: BASARISIZ\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    printf("\nBasarili test: %d/5\n", basarili);
    printf("UART geri dongu testi tamamlandi.\n");
}