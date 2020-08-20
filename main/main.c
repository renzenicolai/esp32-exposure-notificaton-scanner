#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#define GATTC_TAG "Beacon scanner"

static esp_ble_scan_params_t ble_scan_params = {
	.scan_type              = BLE_SCAN_TYPE_ACTIVE,
	.own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
	.scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
	.scan_interval          = 0x50,
	.scan_window            = 0x30,
	.scan_duplicate         = BLE_SCAN_DUPLICATE_DISABLE
};

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
	uint8_t *adv_name = NULL;
	uint8_t adv_name_len = 0;
	switch (event) {
		case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
			esp_ble_gap_start_scanning(0); // Duration of 0 means the scan will not automatically end
			break;
		}
		case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
			//scan start complete event to indicate scan start successfully or failed
			if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
				ESP_LOGE(GATTC_TAG, "scan start failed, error status = %x", param->scan_start_cmpl.status);
				break;
			}
			ESP_LOGI(GATTC_TAG, "scan start success");

			break;
		case ESP_GAP_BLE_SCAN_RESULT_EVT: {
			esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
			switch (scan_result->scan_rst.search_evt) {
				case ESP_GAP_SEARCH_INQ_RES_EVT:
					
					if (scan_result->scan_rst.adv_data_len > 0) {
						bool     isExposureNotification = false;
						uint8_t* rpi = NULL; // Rolling Proximity Identifier
						uint8_t* aem = NULL; // Associated Encrypted Metadata
						
						for (uint32_t i = 0; i < scan_result->scan_rst.adv_data_len;) {
							uint8_t length = scan_result->scan_rst.ble_adv[i++];
							uint8_t type = scan_result->scan_rst.ble_adv[i++];
							if (length > 1) {
								uint8_t* content = &scan_result->scan_rst.ble_adv[i];
								i += length-1;
								if (type == 0x03) { // Service UUID
									if (length == 0x03) { // Expect 2 bytes of content
										if ((content[0] == 0x6F) && (content[1] == 0xFD)) { // Exposure notification service
											isExposureNotification = true;
										}
									}
								} else if (type == 0x16) { // Service DATA
									if (length == 0x17) { // Expect 22 bytes of content
										if ((content[0] == 0x6F) && (content[1] == 0xFD)) { // Exposure notification service
											content += 2; // Length of service identifier is 2
											rpi = content;
											content += 16; // Length of RPI is 16
											aem = content; // (length of AEM is 4)
										}
									}
								}
							}
						}
						
						if ((isExposureNotification) && (rpi != NULL) && (aem != NULL)) {
							printf("{\"mac\":\"");
							for (uint8_t i = 0; i < 6; i++) {
								printf("%02x", scan_result->scan_rst.bda[i]);
								if (i < 5) printf(":");
							}
							printf("\", \"rssi\":%d, \"rpi\":\"", scan_result->scan_rst.rssi);
							for (uint8_t i = 0; i < 16; i++) printf("%02x", rpi[i]);
							printf("\", \"aem\":\"");
							for (uint8_t i = 0; i < 4; i++) printf("%02x", aem[i]);
							printf("\"}\n");
						}
					}
					portYIELD();
					break;
				case ESP_GAP_SEARCH_INQ_CMPL_EVT:
					// End of scan, should not occur
					break;
				default:
					// Unhandled scan event type
					break;
			}
			break;
		}
		break;
	default:
		// Unhandled BLE event type
		break;
	}
}

void app_main(void)
{
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK( ret );

	ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	ret = esp_bt_controller_init(&bt_cfg);
	if (ret) {
		ESP_LOGE(GATTC_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
		return;
	}

	ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
	if (ret) {
		ESP_LOGE(GATTC_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
		return;
	}

	ret = esp_bluedroid_init();
	if (ret) {
		ESP_LOGE(GATTC_TAG, "%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
		return;
	}

	ret = esp_bluedroid_enable();
	if (ret) {
		ESP_LOGE(GATTC_TAG, "%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
		return;
	}

	//register the  callback function to the gap module
	ret = esp_ble_gap_register_callback(esp_gap_cb);
	if (ret){
		ESP_LOGE(GATTC_TAG, "%s gap register failed, error code = %x\n", __func__, ret);
		return;
	}
	
	esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
	if (local_mtu_ret){
		ESP_LOGE(GATTC_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
	}
	
	esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
	if (scan_ret){
		ESP_LOGE(GATTC_TAG, "set scan params error, error code = %x", scan_ret);
	}
}

