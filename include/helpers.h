#pragma once

#include <stdio.h>

namespace application {

class Helpers {
 public:
  static bool floatToInts(const float val, int &int_part, int &frac_part,
                          const int frac_size) {
    if (frac_size <= 0) {
      return false;
    }

    int_part = static_cast<int>(val);
    float temp_val = (val - int_part);
    for (int i = 0; i < frac_size + 1; i++) {
      temp_val *= 10;
    }
    frac_part = static_cast<int>(temp_val);
    if ((frac_part % 10) >= 5) {
      frac_part++;
    }
    frac_part /= 10;

    return true;
  }

  static bool floatToChar(float val, int frac_size, char *buffer) {
    bool if_converted = false;
    int val_int, val_frac;

    if (floatToInts(val, val_int, val_frac, frac_size)) {
      sprintf(buffer, "%d.%d", val_int, val_frac);
      if_converted = true;
    }

    return if_converted;
  }

  Helpers(){};
};

}  // namespace application
