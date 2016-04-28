/* http_get - Retrieves a web page over HTTP GET.
 *
 * See http_get_ssl for a TLS-enabled version.
 *
 * This sample code is in the public domain.,
 */
#include "espressif/esp_common.h"
#include "esp/uart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "http.h"
#include "dht.h"

#include "ssid_config.h"

#define WEB_SERVER "192.168.1.49"

void http_get_task(void *pvParameters)
{
    char* post_request = NULL;
    char* get_request = NULL;

    uint8_t dht_gpio = 4;
    int8_t humidity = 0;
    int8_t temperature = 0;
    char post_body[100];

    gpio_set_pullup(dht_gpio, false, false);

    while(1) {

        if (dht_fetch_data(dht_gpio, &humidity, &temperature)) {

            sprintf(post_body, "{\"DHT11_Measurement\": {\"Humidity\":%i, \"Temperature\":%i}}", humidity, temperature);

            get_request = http_get_request(WEB_SERVER, "/");

            post_request = http_post_request(WEB_SERVER, "/", 
                                                "application/json;charset=utf-8", (char*) post_body);

            http_process_request(WEB_SERVER, "80", get_request, 1024);
        }

        printf("\r\n");

        for(int countdown = 10; countdown >= 0; countdown--) {
            printf("%d... ", countdown);
            vTaskDelay(1200 / portTICK_RATE_MS);
        }

        printf("\r\nStarting again!\r\n");
    }
}

void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };

    /* required to call wifi_set_opmode before station_set_config */
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    xTaskCreate(&http_get_task, (signed char *)"http_get_task", 4096, NULL, 2, NULL);
}

