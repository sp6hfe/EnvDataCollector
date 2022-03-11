#pragma once

#include <WString.h>

namespace interfaces {

class ISensor {
 public:
  virtual bool init() = 0;
  virtual bool measure(unsigned long timestamp) = 0;

  virtual bool newData() const = 0;
  virtual float getData() = 0;
  virtual String getDataId() const = 0;

  virtual String getName() const = 0;
  virtual String getUnit() const = 0;
};

}  // namespace interfaces