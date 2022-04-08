#pragma once

#include <Stream.h>
#include <WString.h>

#include <cstdint>
#include <vector>

#include "IConfigurator.h"
#include "IDataUploader.h"
#include "ISensor.h"
#include "ISystem.h"
#include "IUi.h"
#include "IWiFi.h"

namespace application {

class Application {
 public:
  enum class StackedExecutionResult {
    FAILED,
    SOME_FAILED,
    OK,
  };

 private:
  enum class OpMode {
    RESTART,
    DATA_GATHERING,
    DATA_LOGGING,
    DATA_UPLOADING,
    IDLE,
    CONFIG,
  };

  Stream &console;
  interfaces::ISystem &system;
  std::vector<interfaces::ISensor *> sensorSet;
  std::vector<interfaces::IDataUploader *> uploaderSet;
  interfaces::IUi *ui;
  interfaces::IConfigurator *configurator;

  OpMode opMode = OpMode::RESTART;
  uint8_t interMeasurementsDelaySec = 60;

  unsigned long lastDataGatheringMillis = 0;

  StackedExecutionResult gatherData(unsigned long dataGatheringEnterMillis);
  void logData();
  StackedExecutionResult uploadData();

 public:
  bool registerSensor(interfaces::ISensor *newSensor);
  bool registerUploader(interfaces::IDataUploader *newUploader);
  bool registerUi(interfaces::IUi *newUi);
  bool registerConfigurator(interfaces::IConfigurator *newConfigurator);
  void setInterMeasurementsDelay(uint8_t seconds);
  bool setup();
  void loop(unsigned long loopEnterMillis);

  explicit Application(Stream &console_, interfaces::ISystem &system_)
      : console(console_), system(system_) {}
};

}  // namespace application