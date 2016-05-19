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
#include <string.h>

#include "ssid_config.h"

#define WEB_SERVER "jsonplaceholder.typicode.com"
#define USER_AGENT "esp-open-rtos/0.1 esp8266"

void get_callback(int status_code, char* body) {
    if (status_code < 400) {
        printf("\r\n============== PARSED RESPONSE ================\r\n");
        printf("\r\n%s\r\n", body);
    } else {
        printf("Something went wrong... \r\n");
    }
}

void http_get_task(void *pvParameters) {

    // Delays creation of request text so the memory doesn't get garbled by other processes...
    //
    //      WTF!?!?!?!?
    //
    vTaskDelay(3000 / portTICK_RATE_MS);

    const char get_req[] = "GET /posts/1 HTTP/1.1";

    char* get_headers[] = {
        "Host: " WEB_SERVER,
        "User-Agent: " USER_AGENT,
        "Connection: Close"
    };

    char* request = http_assemble_request(get_req, get_headers, 3, "");

    while(1) {
        
        printf("\r\n");

        for(int countdown = 5; countdown > 0; countdown--) {
            printf("%d... ", countdown);
            vTaskDelay(1000 / portTICK_RATE_MS);
        }

        printf("\r\n\r\n============== SENDING REQUEST ================\r\n");
        printf("%s\r\n", request);

        http_process_request(WEB_SERVER, "80", request, 2048, &get_callback);
    }
}

void post_callback(int status_code, char* body) {
    if (status_code < 400) {
        printf("\r\n============== PARSED RESPONSE ================\r\n");
        printf("\r\n%s\r\n", body);
    } else {
        printf("Something went wrong... \r\n");
    }
}

void http_post_task(void *pvParameters)
{
    const char post_req[] = "POST /posts HTTP/1.1";
    char post_body[100];
    char content_length[5];
    char* post_headers[5];
    char* request;

    post_headers[0] = "Host: " WEB_SERVER;
    post_headers[1] = "User-Agent: " USER_AGENT;
    post_headers[2] = "Connection: Close";
    post_headers[3] = "Content-Type: application/json;charset=utf-8";

    while(1) {

        printf("\r\n");

        for(int countdown = 5; countdown > 0; countdown--) {
            printf("%d... ", countdown);
            vTaskDelay(1200 / portTICK_RATE_MS);
        }

        sprintf(post_body, "{\"Sample_Post_Body\": {\"foo\":%i, \"bar\":%i}}", 8, 9);

        snprintf(content_length, 4, "%d", strlen(post_body));

        post_headers[4] = http_assemble_header("Content-Length", ": ", content_length);

        request = http_assemble_request(post_req, post_headers, 5, post_body);

        printf("\r\n\r\n============== SENDING REQUEST ================\r\n");
        printf("%s\r\n", request);

        http_process_request(WEB_SERVER, "80", request, 2048, &post_callback);
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
    // xTaskCreate(&http_post_task, (signed char *)"http_post_task", 4096, NULL, 2, NULL);
}

