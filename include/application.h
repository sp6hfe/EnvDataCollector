#pragma once

#include <Stream.h>

#include "ISensor.h"
#include "httpUploader.h"
#include "wifiCore.h"

namespace application {

class Application {
 private:
  enum class OpMode {
    RESTART,
    MEASUREMENTS,
    CONFIG,
  };

  OpMode opMode = OpMode::RESTART;
  Stream &console;
  wrappers::WifiCore &wifiCore;
  interfaces::ISensor &sensorTemperature;
  interfaces::ISensor &sensorHumidity;
  interfaces::ISensor &sensorPressureRaw;
  wrappers::HttpUploader dataUploader;

  void root_web_page();
  void config_web_page();
  void restart_web_page();
  void configure_web_server();
  void log_measurements();
  bool upload_link_ready(const char *wifi_ssid, const char *wifi_passphrase,
                         const uint8_t timeout_sec);
  bool upload_data();

 public:
  bool setup();
  void loop();

  explicit Application(Stream &console_, wrappers::WifiCore &wifiCore_,
                       interfaces::ISensor &sensorTemperature_,
                       interfaces::ISensor &sensorHumidity_,
                       interfaces::ISensor &sensorPressureRaw_)
      : console(console_),
        wifiCore(wifiCore_),
        sensorTemperature(sensorTemperature_),
        sensorHumidity(sensorHumidity_),
        sensorPressureRaw(sensorPressureRaw_),
        dataUploader(console, wifiCore){};
};

}  // namespace application