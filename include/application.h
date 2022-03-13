#pragma once

#include <Stream.h>
#include <WString.h>
#include <cstdint>
#include <vector>

#include "IDataUploader.h"
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

  Stream &console;
  interfaces::ISystem &system;
  interfaces::IWiFi &wifi;
  interfaces::IWebServer &webServer;
  std::vector<interfaces::ISensor *> sensorSet;
  std::vector<interfaces::IDataUploader *> uploaderSet;

  OpMode opMode = OpMode::RESTART;
  uint8_t interMeasurementsDelaySec = 60;

  String wifiSsid = "notConfigured";
  String wifiPass = "noPass";
  uint8_t wifiConnectionTimeoutSec = 10;

  String apSsid = "EnvDataCollector";
  String apPass = "noPass";

  void webPageRoot();
  void webPageConfig();
  void webPageRestart();
  void webserverConfig();
  bool uploadLinkReady();
  bool logAndUpload(bool logOnly);

 public:
  bool registerSensor(interfaces::ISensor *newSensor);
  bool registerUploader(interfaces::IDataUploader *newUploader);
  void setInterMeasurementsDelay(uint8_t seconds);
  void setWifiConnectionParams(const char *ssid, const char *pass,
                               uint8_t timeoutSec);
  void setApConnectionParams(const char *ssid, const char *pass);
  bool setup();
  void loop(unsigned long loopEnterMillis);

  explicit Application(Stream &console_, interfaces::ISystem &system_,
                       interfaces::IWiFi &wifi_,
                       interfaces::IWebServer &webServer_)
      : console(console_),
        system(system_),
        wifi(wifi_),
        webServer(webServer_) {}
};

}  // namespace application