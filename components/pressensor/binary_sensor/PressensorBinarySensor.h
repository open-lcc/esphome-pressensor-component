//
// Created by Magnus Nordlander on 2023-10-31.
//

#ifndef ESPHOME_BIANCA_PRESSENSORBINARYSENSOR_H
#define ESPHOME_BIANCA_PRESSENSORBINARYSENSOR_H

#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/helpers.h"
#include "../Pressensor.h"

namespace esphome {
    namespace pressensor {
        class PressensorBinarySensor
                : public esphome::Component, public esphome::Parented<Pressensor> {
        public:
            void setup() {
                get_parent()->add_on_status_callback([this](PressensorStatus msg) { this->handleMessage(msg); });
            }

            void handleMessage(PressensorStatus message) {
                if (connected_ != nullptr) {
                    connected_->publish_state(message.connected);
                }
            }

            void set_connected(esphome::binary_sensor::BinarySensor *sens) { connected_ = sens; }
        protected:
            esphome::binary_sensor::BinarySensor *connected_{nullptr};
        };
    }
}

#endif //ESPHOME_BIANCA_PRESSENSORBINARYSENSOR_H
