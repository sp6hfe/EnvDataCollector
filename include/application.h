#pragma once

#include <Stream.h>
#include <vector>

#include "IDataUploader.h"
#include "IHttp.h"
#include "ISensor.h"
#include "ISystem.h"
#include "IWebServer.h"
#include "IWiFi.h"

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
  interfaces::ISystem &system;
  interfaces::IWiFi &wifi;
  interfaces::IWebServer &webServer;
  interfaces::IHttp &http;
  std::vector<interfaces::ISensor *> sensorSet;
  std::vector<interfaces::IDataUploader *> uploaderSet;

  void webPageRoot();
  void webPageConfig();
  void webPageRestart();
  void webserverConfig();
  bool uploadLinkReady(const char *wifiSsid, const char *wifiPassphrase,
                       const uint8_t timeoutSec);
  bool logAndUpload(bool logOnly);

 public:
  bool registerSensor(interfaces::ISensor *newSensor);
  bool registerUploader(interfaces::IDataUploader *newUploader);
  bool setup();
  void loop(unsigned long loopEnterMillis);

  explicit Application(Stream &console_, interfaces::ISystem &system_,
                       interfaces::IWiFi &wifi_,
                       interfaces::IWebServer &webServer_,
                       interfaces::IHttp &http_)
      : console(console_),
        system(system_),
        wifi(wifi_),
        webServer(webServer_),
        http(http_) {}
};

}  // namespace application