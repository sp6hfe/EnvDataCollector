#pragma once

#include <Stream.h>
#include <vector>

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
  wrappers::HttpUploader dataUploader;
  std::vector<interfaces::ISensor *> sensorSet;

  void root_web_page();
  void config_web_page();
  void restart_web_page();
  void configure_web_server();
  void log_measurements();
  bool upload_link_ready(const char *wifi_ssid, const char *wifi_passphrase,
                         const uint8_t timeout_sec);
  bool upload_data();

 public:
  bool registerSensor(interfaces::ISensor *sensor);
  bool setup();
  void loop(unsigned long loop_enter_millis);

  explicit Application(Stream &console_, wrappers::WifiCore &wifiCore_)
      : console(console_),
        wifiCore(wifiCore_),
        dataUploader(console, wifiCore){};
};

}  // namespace application