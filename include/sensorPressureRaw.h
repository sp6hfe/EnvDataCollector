#pragma once

#include "hwBme280.h"
#include "sensorBase.h"

namespace sensors {

class SensorPressureRaw : public sensors::SensorBase {
 private:
  wrappers::HwBme280 &bme280;

 public:
  bool init() final { return this->bme280.init(); }

  bool measure(unsigned long timestamp) final {
    return this->bme280.measure(timestamp);
  }

  bool newData() const final { return this->bme280.isNewPressureRaw(); }

  float getData() final { return this->bme280.getPressureRaw(); }

  explicit SensorPressureRaw(wrappers::HwBme280 &bme280_, String name_,
                             String unit_, String dataId_)
      : sensors::SensorBase(name_, unit_, dataId_), bme280(bme280_) {}

  virtual ~SensorPressureRaw() {}
};

}  // namespace sensors