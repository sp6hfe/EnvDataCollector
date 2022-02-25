#pragma once

#include <ISensor.h>
#include "hwBme280.h"

namespace sensors
{
    class SensorTemperature : public interfaces::ISensor
    {
    private:
        wrappers::HwBme280 &bme280;
        String name;

    public:
        bool init() override { return bme280.init(); };
        bool measure() override { return bme280.measure(); }
        bool newValue() const override { return bme280.isNewTemperature(); };
        float getValue() override { return bme280.getTemperature(); }
        String getName() const override { return name; };

        SensorTemperature(wrappers::HwBme280 &bme280_, String name_ = "no_temeperature_sensor_name") : bme280(bme280_), name(name_){};
    };
} // namespace wrappers