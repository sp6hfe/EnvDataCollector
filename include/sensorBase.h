#pragma once

#include <ISensor.h>
#include <WString.h>

namespace sensors {

class SensorBase : public interfaces::ISensor {
 protected:
  String name;
  String unit;

 public:
  // partially implemented interface
  String getName() const final { return this->name; }
  String getUnit() const final { return this->unit; }

  SensorBase(String name_, String unit_) : name(name_), unit(unit_){};
  virtual ~SensorBase(){};
};

}  // namespace sensors
