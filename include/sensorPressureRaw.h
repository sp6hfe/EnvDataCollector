#pragma once

#include <ISensor.h>
#include "hwBme280.h"

namespace sensors {

class SensorPressureRaw : public interfaces::ISensor {
 private:
  wrappers::HwBme280 &bme280;
  String name;

 public:
  bool init() override { return this->bme280.init(); }
  bool measure() override { return this->bme280.measure(); }
  bool newValue() const override { return this->bme280.isNewPressureRaw(); }
  float getValue() override { return this->bme280.getPressureRaw(); }
  String getName() const override { return this->name; }

  explicit SensorPressureRaw(wrappers::HwBme280 &bme280_,
                             String name_ = "no_pressure_raw_sensor_name")
      : bme280(bme280_), name(name_){};
};

}  // namespace sensors