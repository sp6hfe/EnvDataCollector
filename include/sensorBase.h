#pragma once

#include <ISensor.h>
#include <WString.h>

namespace sensors {

class SensorBase : public interfaces::ISensor {
 protected:
  String name;
  String unit;
  String dataId;

 public:
  // partially implemented interface
  String getName() const final { return this->name; }
  String getUnit() const final { return this->unit; }
  String getDataId() const final { return this->dataId; }

  SensorBase(String name_, String unit_, String dataId_)
      : name(name_), unit(unit_), dataId(dataId_) {}
  virtual ~SensorBase() {}
};

}  // namespace sensors
