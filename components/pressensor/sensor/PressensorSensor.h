//
// Created by Magnus Nordlander on 2023-10-29.
//

#ifndef SMART_LCC_PRESSENSORSENSOR_H
#define SMART_LCC_PRESSENSORSENSOR_H

#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/helpers.h"
#include "../Pressensor.h"

namespace esphome {
    namespace pressensor {
        class PressensorSensor
                : public esphome::Component, public esphome::Parented<Pressensor> {
        public:
            void setup() {
                get_parent()->add_on_pressure_callback([this](int16_t msg) { this->handleMessage(msg); });
            }

            void handleMessage(int16_t pressureInMillibar) {
                float pressureInPascal = pressureInMillibar * 100.f;
                if (should_set(pressure_, pressureInPascal, 10000)) {
                    pressure_->publish_state(pressureInPascal);
                }
            }

            bool should_set(esphome::sensor::Sensor *sensor, float value, float sigma = 0.1)
            {
                return sensor != nullptr && (!sensor->has_state() || fabs(sensor->get_state() - value) >= sigma);
            }

            void set_pressure(esphome::sensor::Sensor *sens) { pressure_ = sens; }
        protected:
            esphome::sensor::Sensor *pressure_{nullptr};

        };
    }
}

#endif //SMART_LCC_PRESSENSORSENSOR_H
