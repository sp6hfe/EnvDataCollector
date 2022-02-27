#pragma once

#include <ISensor.h>
#include "hwBme280.h"

namespace sensors {

class SensorHumidity : public interfaces::ISensor {
 private:
  wrappers::HwBme280 &bme280;
  String name;

 public:
  bool init() override { return this->bme280.init(); }
  bool measure() override { return this->bme280.measure(); }
  bool newValue() const override { return this->bme280.isNewHumidity(); }
  float getValue() override { return this->bme280.getHumidity(); }
  String getName() const override { return this->name; }

  explicit SensorHumidity(wrappers::HwBme280 &bme280_,
                          String name_ = "no_humidity_sensor_name")
      : bme280(bme280_), name(name_){};
};

}  // namespace sensors