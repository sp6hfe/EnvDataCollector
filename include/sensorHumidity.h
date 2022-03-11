#pragma once

#include "hwBme280.h"
#include "sensorBase.h"

namespace sensors {

class SensorHumidity : public sensors::SensorBase {
 private:
  wrappers::HwBme280 &bme280;

 public:
  bool init() final { return this->bme280.init(); }

  bool measure(unsigned long timestamp) final {
    return this->bme280.measure(timestamp);
  }

  bool newValue() const final { return this->bme280.isNewHumidity(); }

  float getValue() final { return this->bme280.getHumidity(); }

  explicit SensorHumidity(wrappers::HwBme280 &bme280_, String name_,
                          String unit_)
      : sensors::SensorBase(name_, unit_), bme280(bme280_){};

  virtual ~SensorHumidity(){};
};

}  // namespace sensors