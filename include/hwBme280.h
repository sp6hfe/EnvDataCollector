#pragma once

#include <Adafruit_BME280.h>

namespace wrappers {

class HwBme280 {
 private:
  static constexpr unsigned long MIN_DELAY_BETWEEN_MEASUREMENTS_MS = 500;

  Adafruit_BME280 bme;
  bool is_init = false;

  unsigned long last_measurement_timestamp = 0;
  float temperature = 0.0f;
  float humidity = 0.0f;
  float pressure_raw = 0.0f;
  bool new_temperature = false;
  bool new_humidity = false;
  bool new_pressure_raw = false;

  unsigned long msSinceLastMeasurement(unsigned long current_timestamp,
                                       unsigned long last_timestamp) {
    // no overflow
    if (current_timestamp >= last_timestamp) {
      return current_timestamp - last_timestamp;
    }

    // with overflow
    unsigned long maxVal = ~0;
    return ((maxVal - last_timestamp) + current_timestamp);
  }

 public:
  bool init(uint8_t address = BME280_ADDRESS_ALTERNATE) {
    this->is_init = false;

    if (this->bme.begin(address)) {
      this->bme.setSampling(
          Adafruit_BME280::MODE_FORCED, Adafruit_BME280::SAMPLING_X1,
          Adafruit_BME280::SAMPLING_X1, Adafruit_BME280::SAMPLING_X1,
          Adafruit_BME280::FILTER_OFF, Adafruit_BME280::STANDBY_MS_0_5);
      this->is_init = true;
    }

    return this->is_init;
  }

  bool isInit() { return this->is_init; }

  bool measure(unsigned long timestamp) {
    bool if_measured = false;

    // Since all parameters are captured within single measurement and each
    // of them is handled by separate abstract sensor (where each of which is
    // performing its own measurement) a time guard (sufficient enough) is added
    // here in order to not perform multiple measurements leading to unwanted
    // sensor self-heating and time/power waste.
    if (this->msSinceLastMeasurement(timestamp,
                                     this->last_measurement_timestamp) >=
        this->MIN_DELAY_BETWEEN_MEASUREMENTS_MS) {
      this->new_temperature = false;
      this->new_humidity = false;
      this->new_pressure_raw = false;

      if (this->is_init && this->bme.takeForcedMeasurement()) {
        this->temperature = this->bme.readTemperature();
        this->new_temperature = true;
        this->humidity = this->bme.readHumidity();
        this->new_humidity = true;
        this->pressure_raw = this->bme.readPressure() / 100.0F;
        this->new_pressure_raw = true;
        this->last_measurement_timestamp = timestamp;
        if_measured = true;
      }
    } else {
      // In case of repeated measurement issued having not all the data
      // read from the initial measurement yet (the one which was performed due
      // to long time of sensor inactivity) a measurement confirmation is
      // returned to satisfy abstract sensors querying the same HW for
      // measurements multiple times in a short period of time.
      if (this->new_temperature || this->new_humidity ||
          this->new_pressure_raw) {
        if_measured = true;
      }
    }

    return if_measured;
  }

  float getTemperature() {
    this->new_temperature = false;
    return this->is_init ? this->temperature : 0.0f;
  }

  bool isNewTemperature() { return this->new_temperature; }

  float getHumidity() {
    this->new_humidity = false;
    return this->is_init ? this->humidity : 0.0f;
  }

  bool isNewHumidity() { return this->humidity; }

  float getPressureRaw() {
    this->new_pressure_raw = false;
    return this->is_init ? this->pressure_raw : 0.0f;
  }

  bool isNewPressureRaw() { return this->pressure_raw; }

  HwBme280(){};
};

}  // namespace wrappers