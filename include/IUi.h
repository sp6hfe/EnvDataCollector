#pragma once

namespace interfaces {

enum class UiDeviceState {
  IDLE,           /* no operation is performed */
  INIT,           /* device initialization ongoing */
  DATA_GATHERING, /* data gathering ongoing */
  DATA_UPLOADING, /* data uploading ongoing */
  ERROR,          /* generic error */
};

class IUi {
 public:
  virtual void setState(UiDeviceState state) = 0;
  virtual void perform() = 0;
};
}  // namespace interfaces
