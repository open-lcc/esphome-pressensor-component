#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/sensor/sensor.h"

#ifdef USE_ESP32

#include <esp_gattc_api.h>

#define PRESSENSOR_PRESSURE_SERVICE_UUID "873ae82a-4c5a-4342-b539-9d900bf7ebd0"
#define PRESSENSOR_PRESSURE_CHARACTERISTIC_UUID "873ae82b-4c5a-4342-b539-9d900bf7ebd0"
#define PRESSENSOR_PRESSURE_CLIENT_CHARACTERISTIC 0x2902

namespace esphome {
    namespace pressensor {
        static const char* TAG = "PRS";


    class Pressensor : public esphome::Component, public esphome::ble_client::BLEClientNode {
        public:
            void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
            {
                switch (event) {
                    case ESP_GATTC_OPEN_EVT: {
                        ESP_LOGD(TAG, "ESP_GATTC_OPEN_EVT");
                        conn_id_ = param->open.conn_id;
                        break;
                    }
                    case ESP_GATTC_SEARCH_CMPL_EVT: {
                        ESP_LOGD(TAG, "Registering for notifications");

                        auto *chr = this->parent_->get_characteristic(
                                esphome::esp32_ble::ESPBTUUID::from_raw(PRESSENSOR_PRESSURE_SERVICE_UUID),
                                esphome::esp32_ble::ESPBTUUID::from_raw(PRESSENSOR_PRESSURE_CHARACTERISTIC_UUID)
                        );
                        if (chr == nullptr) {
                            ESP_LOGW(TAG, "No control service found at device, not an Anova..?");
                            break;
                        }
                        this->char_handle_ = chr->handle;

                        auto status = esp_ble_gattc_register_for_notify(this->parent_->get_gattc_if(), this->parent_->get_remote_bda(),
                                                                        chr->handle);
                        if (status) {
                            ESP_LOGW(TAG, "esp_ble_gattc_register_for_notify failed, status=%d", status);
                        }
                        break;
                    }
                    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
                        ESP_LOGD(TAG, "Registered for notifications, getting client characteristic");

                        auto *chr = this->parent_->get_config_descriptor(
                                this->char_handle_
                        );
                        if (chr == nullptr) {
                            ESP_LOGW(TAG, "No client config descriptor found");
                            break;
                        }

                        uint16_t notify_en = 1;
                        esp_err_t status =
                            esp_ble_gattc_write_char_descr(this->parent_->get_gattc_if(), this->conn_id_, chr->handle, sizeof(notify_en),
                                         (uint8_t *) &notify_en, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
                        if (status) {
                            ESP_LOGW(TAG, "esp_ble_gattc_write_char_descr error, status=%d", status);
                        }
                        break;
                    }
                    case ESP_GATTC_NOTIFY_EVT: {
                        if (param->notify.handle != this->char_handle_)
                            break;

                        if (param->notify.value_len >= 2) {
                            int16_t val = (param->notify.value[0] << 8) + param->notify.value[1];
                            this->on_pressure_callback_.call(val);
                        }

                        break;
                    }
                    default:
                        ESP_LOGD(TAG, "GATTC Event incoming, type %u", event);
                }
            }

            void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
            {
            }

            void loop()
            {

            }

            void add_on_pressure_callback(std::function<void(int16_t)> callback) {
                this->on_pressure_callback_.add(std::move(callback));
            }

        protected:
            uint16_t char_handle_;
            uint16_t conn_id_;

            CallbackManager<void(int16_t)> on_pressure_callback_;
        };
    }
}

#endif