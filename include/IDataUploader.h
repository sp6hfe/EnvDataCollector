#pragma once

namespace interfaces {

class IDataUploader {
 public:
  virtual String getName() const = 0;
  virtual void clearData() = 0;
  virtual bool addData(const String name, const float value) = 0;
  virtual bool upload() = 0;
};

}  // namespace interfaces