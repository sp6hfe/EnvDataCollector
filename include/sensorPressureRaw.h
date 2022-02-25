#pragma once

#include <ISensor.h>
#include "hwBme280.h"

namespace sensors
{
    class SensorPressureRaw : public interfaces::ISensor
    {
    private:
        wrappers::HwBme280 &bme280;
        String name;

    public:
        bool init() override { return bme280.init(); };
        bool measure() override { return bme280.measure(); }
        bool newValue() const override { return bme280.isNewPressureRaw(); };
        float getValue() override { return bme280.getPressureRaw(); }
        String getName() const override { return name; };

        SensorPressureRaw(wrappers::HwBme280 &bme280_, String name_ = "no_pressure_raw_sensor_name") : bme280(bme280_), name(name_){};
    };
} // namespace wrappers