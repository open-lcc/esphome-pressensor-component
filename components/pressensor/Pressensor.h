#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/sensor/sensor.h"

#ifdef USE_ESP32

#include <esp_gattc_api.h>

#define PRESSENSOR_PRESSURE_SERVICE_UUID "873ae82a-4c5a-4342-b539-9d900bf7ebd0"
#define PRESSENSOR_PRESSURE_CHARACTERISTIC_UUID "873ae82b-4c5a-4342-b539-9d900bf7ebd0"

#define BATTERY_SERVICE_UUID "0000180f-0000-1000-8000-00805f9b34fb"
#define BATTERY_CHARACTERISTIC_UUID "00002a19-0000-1000-8000-00805f9b34fb"

namespace esphome {
    namespace pressensor {
        static const char* TAG = "PRS";

        class PressensorStatus {
        public:
            int16_t pressureMbar = 0;
            uint8_t batteryLevel = 0;
            bool connected = false;
        };

    class Pressensor : public esphome::Component, public esphome::ble_client::BLEClientNode {
        public:
            void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
            {
                switch (event) {
                    case ESP_GATTC_OPEN_EVT: {
                        ESP_LOGD(TAG, "ESP_GATTC_OPEN_EVT");
                        conn_id_ = param->open.conn_id;

                        pressensorStatus.connected = true;
                        this->on_status_callback_.call(pressensorStatus);

                        break;
                    }
                    case ESP_GATTC_SEARCH_CMPL_EVT: {
                        ESP_LOGD(TAG, "Registering for notifications");

                        auto *pressureChar = this->parent_->get_characteristic(
                                esphome::esp32_ble::ESPBTUUID::from_raw(PRESSENSOR_PRESSURE_SERVICE_UUID),
                                esphome::esp32_ble::ESPBTUUID::from_raw(PRESSENSOR_PRESSURE_CHARACTERISTIC_UUID)
                        );
                        if (pressureChar == nullptr) {
                            ESP_LOGW(TAG, "No control service found at device, not an Anova..?");
                            break;
                        }
                        this->pressure_char_handle_ = pressureChar->handle;

                        auto pressStatus = esp_ble_gattc_register_for_notify(this->parent_->get_gattc_if(), this->parent_->get_remote_bda(),
                                                                        pressureChar->handle);
                        if (pressStatus) {
                            ESP_LOGW(TAG, "esp_ble_gattc_register_for_notify failed, status=%d", pressStatus);
                        }

                        auto *batteryChar = this->parent_->get_characteristic(
                                esphome::esp32_ble::ESPBTUUID::from_raw(BATTERY_SERVICE_UUID),
                                esphome::esp32_ble::ESPBTUUID::from_raw(BATTERY_CHARACTERISTIC_UUID)
                        );
                        if (batteryChar == nullptr) {
                            ESP_LOGW(TAG, "No control service found at device, not an Anova..?");
                            break;
                        }
                        this->battery_char_handle_ = batteryChar->handle;

                        auto battStatus = esp_ble_gattc_register_for_notify(this->parent_->get_gattc_if(), this->parent_->get_remote_bda(),
                                                                            batteryChar->handle);
                        if (battStatus) {
                            ESP_LOGW(TAG, "esp_ble_gattc_register_for_notify failed, status=%d", battStatus);
                        }

                        auto readStatus = esp_ble_gattc_read_char(this->parent_->get_gattc_if(), this->conn_id_, battery_char_handle_, ESP_GATT_AUTH_REQ_NONE);

                        if (readStatus) {
                            ESP_LOGW(TAG, "esp_ble_gattc_read_char failed, status=%d", readStatus);
                        }

                        break;
                    }
                    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
                        ESP_LOGD(TAG, "Registered for notifications, getting client characteristic");

                        auto *chr = this->parent_->get_config_descriptor(
                                param->reg_for_notify.handle
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
                    case ESP_GATTC_READ_CHAR_EVT: {
                        ESP_LOGD(TAG, "Read event");

                        if (param->read.handle != this->battery_char_handle_)
                            break;

                        if (param->read.value_len == 1) {
                            pressensorStatus.batteryLevel = param->read.value[0];
                            this->on_status_callback_.call(pressensorStatus);
                        } else {
                            ESP_LOGW(TAG, "Unexpected length of battery data: %u", param->read.value_len);
                        }

                        break;
                    }
                    case ESP_GATTC_NOTIFY_EVT: {
                        if (param->notify.handle == this->pressure_char_handle_) {
                            if (param->notify.value_len >= 2) {
                                int16_t val = (param->notify.value[0] << 8) + param->notify.value[1];
                                pressensorStatus.pressureMbar = val;
                                this->on_status_callback_.call(pressensorStatus);
                            }
                        } else if (param->notify.handle == this->battery_char_handle_) {
                            if (param->notify.value_len == 1) {
                                pressensorStatus.batteryLevel = param->notify.value[0];
                                this->on_status_callback_.call(pressensorStatus);
                            } else {
                                ESP_LOGW(TAG, "Unexpected length of battery data: %u", param->read.value_len);
                            }
                        }

                        break;
                    }
                    case ESP_GATTC_CLOSE_EVT: {
                        pressensorStatus.connected = false;
                        this->on_status_callback_.call(pressensorStatus);
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

            void add_on_status_callback(std::function<void(PressensorStatus)> callback) {
                this->on_status_callback_.add(std::move(callback));
            }

        protected:
            uint16_t pressure_char_handle_;
            uint16_t battery_char_handle_;
            uint16_t conn_id_;

            PressensorStatus pressensorStatus;

            CallbackManager<void(PressensorStatus)> on_status_callback_;
        };
    }
}

#endif