#pragma once

#include "hwBme280.h"
#include "sensorBase.h"

namespace sensors {

class SensorTemperature : public sensors::SensorBase {
 private:
  wrappers::HwBme280 &bme280;

 public:
  bool init() override { return this->bme280.init(); }

  bool measure(unsigned long timestamp) override {
    return this->bme280.measure(timestamp);
  }

  bool newValue() const override { return this->bme280.isNewTemperature(); }

  float getValue() override { return this->bme280.getTemperature(); }

  explicit SensorTemperature(wrappers::HwBme280 &bme280_, String name_,
                             String unit_)
      : sensors::SensorBase(name_, unit_), bme280(bme280_){};

  virtual ~SensorTemperature(){};
};

}  // namespace sensors