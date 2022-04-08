#pragma once

#include <Arduino.h>

#include "IUi.h"

namespace ui {

class LedUi : public interfaces::IUi {
 private:
  bool isConfigured = false;
  uint8_t ledPinNo = 0;
  bool ledIdleHigh = false;

 public:
  void begin(int ledPin, bool isIdleHigh) {
    if (ledPin >= 0) {
      this->ledPinNo = static_cast<uint8_t>(ledPin);
      this->ledIdleHigh = isIdleHigh;

      pinMode(this->ledPinNo, OUTPUT);

      if (this->ledIdleHigh) {
        digitalWrite(this->ledPinNo, HIGH);
      } else {
        digitalWrite(this->ledPinNo, LOW);
      }

      isConfigured = true;
    }
  }
  void setState(interfaces::UiDeviceState state) override {}
  void perform() override {}
};
}  // namespace ui
