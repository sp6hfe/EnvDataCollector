#pragma once

#include <ISensor.h>
#include "hwBme280.h"

namespace sensors {

class SensorTemperature : public interfaces::ISensor {
 private:
  wrappers::HwBme280 &bme280;
  String name;

 public:
  bool init() override { return this->bme280.init(); }
  bool measure() override { return this->bme280.measure(); }
  bool newValue() const override { return this->bme280.isNewTemperature(); }
  float getValue() override { return this->bme280.getTemperature(); }
  String getName() const override { return this->name; }

  explicit SensorTemperature(wrappers::HwBme280 &bme280_,
                             String name_ = "no_temeperature_sensor_name")
      : bme280(bme280_), name(name_){};
};

}  // namespace sensors