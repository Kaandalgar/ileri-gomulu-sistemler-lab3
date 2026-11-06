#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_err.h"

#define TX_UART UART_NUM_1
#define RX_UART UART_NUM_2

static void uartlari_baslat(void)
{
    uart_config_t ayar = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT};

    ESP_ERROR_CHECK(
        uart_driver_install(TX_UART, 1024, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(
        uart_driver_install(RX_UART, 1024, 0, 0, NULL, 0));

    ESP_ERROR_CHECK(uart_param_config(TX_UART, &ayar));
    ESP_ERROR_CHECK(uart_param_config(RX_UART, &ayar));

    /* UART1: TX17 kullaniliyor; RX16 baglanmayacak. */
    ESP_ERROR_CHECK(
        uart_set_pin(TX_UART, 17, 16,
                     UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    /* UART2: RX18 kullaniliyor; TX19 baglanmayacak. */
    ESP_ERROR_CHECK(
        uart_set_pin(RX_UART, 19, 18,
                     UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

static void baud_testi(const char *baslik, int alici_baud)
{
    ESP_ERROR_CHECK(uart_set_baudrate(RX_UART, alici_baud));
    vTaskDelay(pdMS_TO_TICKS(100));

    printf("\n--- %s ---\n", baslik);
    printf("Gonderici: 115200 baud\n");
    printf("Alici: %d baud\n", alici_baud);

    int basarili = 0;

    for (int i = 1; i <= 3; i++)
    {
        char mesaj[64];
        uint8_t alinan[64] = {0};

        snprintf(mesaj, sizeof(mesaj),
                 "UART baud testi %d", i);

        int uzunluk = (int)strlen(mesaj);

        ESP_ERROR_CHECK(uart_flush_input(RX_UART));

        int yazilan =
            uart_write_bytes(TX_UART, mesaj, uzunluk);

        ESP_ERROR_CHECK(
            uart_wait_tx_done(TX_UART, pdMS_TO_TICKS(1000)));

        int okunan = uart_read_bytes(
            RX_UART,
            alinan,
            sizeof(alinan) - 1,
            pdMS_TO_TICKS(200));

        printf("\nTest %d\n", i);
        printf("Gonderilen: %s\n", mesaj);
        printf("Alinan bayt sayisi: %d\n", okunan);
        printf("Alinan HEX:");

        for (int j = 0; j < okunan; j++)
        {
            printf(" %02X", (unsigned int)alinan[j]);
        }

        printf("\n");

        if (yazilan == uzunluk &&
            okunan == uzunluk &&
            memcmp(mesaj, alinan, uzunluk) == 0)
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

    printf("Eslesen test: %d/3\n", basarili);
}

void app_main(void)
{
    uartlari_baslat();

    printf("\nLAB 3 - UART BAUD KARSILASTIRMASI\n");
    printf("Baglanti: GPIO17 -> GPIO18\n");

    baud_testi("AYNI BAUD", 115200);
    baud_testi("YANLIS BAUD", 9600);
    baud_testi("AYNI BAUDA DONUS", 115200);

    printf("\nBaud karsilastirmasi tamamlandi.\n");
}