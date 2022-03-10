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

  void webPageRoot();
  void webPageConfig();
  void webPageRestart();
  void webserverConfig();
  bool uploadLinkReady(const char *wifiSsid, const char *wifiPassphrase,
                       const uint8_t timeoutSec);
  bool logAndUpload(bool logOnly);

 public:
  bool registerSensor(interfaces::ISensor *sensor);
  bool setup();
  void loop(unsigned long loopEnterMillis);

  explicit Application(Stream &console_, wrappers::WifiCore &wifiCore_)
      : console(console_),
        wifiCore(wifiCore_),
        dataUploader(console, wifiCore){};
};

}  // namespace application