#include "https.h"

#include <stdio.h>
   #include <string.h>
   #include "esp_log.h"
   #include "esp_http_client.h"
   #include "esp_tls.h"
   #include "esp_wifi.h"
   #include "nvs_flash.h"
   #include "esp_event.h"

   static const char *TAG = "https_request";

   static void wifi_init(void)
   {
       esp_err_t ret = nvs_flash_init();
       if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
           ESP_ERROR_CHECK(nvs_flash_erase());
           ret = nvs_flash_init();
       }
       ESP_ERROR_CHECK(ret);

       ESP_ERROR_CHECK(esp_netif_init());
       ESP_ERROR_CHECK(esp_event_loop_create_default());

       wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
       ESP_ERROR_CHECK(esp_wifi_init(&cfg));
       ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
       ESP_ERROR_CHECK(esp_wifi_start());
   }

   static void https_request(void)
   {
       esp_http_client_config_t config = {
           .url = "https://example.com",
           .method = HTTP_METHOD_GET,
           .cert_pem = NULL, // 如果需要客户端证书验证，可以在这里设置证书
       };

       esp_http_client_handle_t client = esp_http_client_init(&config);
       if (client == NULL) {
           ESP_LOGE(TAG, "Failed to initialize HTTP client");
           return;
       }

       esp_err_t err = esp_http_client_perform(client);
       if (err == ESP_OK) {
           ESP_LOGI(TAG, "HTTP GET status = %d, content_length = %d",
                    esp_http_client_get_status_code(client),
                    esp_http_client_get_content_length(client));
       } else {
           ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
       }

       esp_http_client_cleanup(client);
   }

   void app_main(void)
   {
       wifi_init();
       https_request();
   }
   
int eps32_HTTPS_Init (int set)
{

}

void eps32_HTTPS_task (void *empty)
{

}