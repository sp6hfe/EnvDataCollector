#pragma once

#include <Stream.h>
#include <WString.h>
#include <cstdint>
#include <vector>

#include "IConfigurator.h"
#include "IDataUploader.h"
#include "ISensor.h"
#include "ISystem.h"
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
  std::vector<interfaces::ISensor *> sensorSet;
  std::vector<interfaces::IDataUploader *> uploaderSet;
  interfaces::IConfigurator *configurator;

  OpMode opMode = OpMode::RESTART;
  uint8_t interMeasurementsDelaySec = 60;

  bool logAndUpload(bool logOnly);

 public:
  bool registerSensor(interfaces::ISensor *newSensor);
  bool registerUploader(interfaces::IDataUploader *newUploader);
  bool registerConfigurator(interfaces::IConfigurator *newConfigurator);
  void setInterMeasurementsDelay(uint8_t seconds);
  bool setup();
  void loop(unsigned long loopEnterMillis);

  explicit Application(Stream &console_, interfaces::ISystem &system_)
      : console(console_), system(system_) {}
};

}  // namespace application