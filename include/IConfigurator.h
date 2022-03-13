#pragma once

namespace interfaces {

class IConfigurator {
 public:
  virtual bool start() = 0;
  virtual int perform() = 0;
};

}  // namespace interfaces
