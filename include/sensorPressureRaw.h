#pragma once

#include <ISensor.h>
#include "hwBme280.h"

namespace sensors {

class SensorPressureRaw : public interfaces::ISensor {
 private:
  wrappers::HwBme280 &bme280;
  String name;
  String unit;

 public:
  bool init() override { return this->bme280.init(); }

  bool measure(unsigned long timestamp) override {
    return this->bme280.measure(timestamp);
  }

  bool newValue() const override { return this->bme280.isNewPressureRaw(); }

  float getValue() override { return this->bme280.getPressureRaw(); }

  String getName() const override { return this->name; }

  String getUnit() const override { return this->unit; }

  explicit SensorPressureRaw(wrappers::HwBme280 &bme280_, String name_,
                             String unit_)
      : bme280(bme280_), name(name_), unit(unit_){};
};

}  // namespace sensors