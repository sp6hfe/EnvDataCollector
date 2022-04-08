#include "application.h"

using namespace application;

Application::StackedExecutionResult Application::gatherData(
    unsigned long dataGatheringEnterMillis) {
  StackedExecutionResult result = StackedExecutionResult::FAILED;

  auto gatheredDataCount = 0;

  for (auto sensor : this->sensorSet) {
    if (sensor->measure(dataGatheringEnterMillis)) {
      gatheredDataCount++;
    } else {
      this->console.print("Error gathering data from \"");
      this->console.print(sensor->getName());
      this->console.println("\" source.");
    }
  }

  if (gatheredDataCount > 0) {
    if (gatheredDataCount == this->sensorSet.size()) {
      result = StackedExecutionResult::OK;
    } else {
      result = StackedExecutionResult::SOME_FAILED;
    }
  }

  return result;
}

void Application::logData() {
  bool anyNewDataAvailable = false;

  for (auto sensor : this->sensorSet) {
    if (sensor->newData()) {
      if (!anyNewDataAvailable) {
        anyNewDataAvailable = true;
        this->console.println();
        this->console.print("New data gathered:");
      }

      this->console.println();
      this->console.print(sensor->getName());
      this->console.print(": ");
      this->console.print(sensor->getData());
      this->console.print("[");
      this->console.print(sensor->getUnit());
      this->console.print("]");
    }
  }

  if (!anyNewDataAvailable) {
    this->console.println();
    this->console.print("No new data was gathered.");
  }
}

Application::StackedExecutionResult Application::uploadData() {
  StackedExecutionResult result = StackedExecutionResult::FAILED;
  bool anyNewMeasurement = false;
  auto succeededUploads = 0;

  // clead data and establish upload links
  bool anyUploaderActive = false;
  for (auto uploader : this->uploaderSet) {
    uploader->clearData();
    if (uploader->uploadLinkSetup()) {
      anyUploaderActive = true;
    }
  }

  if (!anyUploaderActive) {
    return result;
  }

  for (auto sensor : this->sensorSet) {
    if (sensor->newData()) {
      for (auto uploader : this->uploaderSet) {
        uploader->addData(sensor->getDataId(), sensor->getData());
      }
      anyNewMeasurement = true;
    }
  }

  if (anyNewMeasurement) {
    this->console.println();
    // upload queued data
    for (auto uploader : this->uploaderSet) {
      if (uploader->upload()) {
        succeededUploads++;
        this->console.print("(+) Data uploaded");
      } else {
        this->console.print("(-) Error uploading");
      }
      this->console.print(" via \"");
      this->console.print(uploader->getName());
      this->console.println("\".");
    }

    if (succeededUploads > 0) {
      if (succeededUploads == this->uploaderSet.size()) {
        result = StackedExecutionResult::OK;
      } else {
        result = StackedExecutionResult::SOME_FAILED;
      }
    }
  } else {
    // nothing to upload (should be detected earlier)
    this->console.println(" no new measurements - check log for errors.");
  }

  return result;
}

bool Application::registerSensor(interfaces::ISensor* newSensor) {
  bool ifSensorRegistered = false;

  if (newSensor) {
    // add only new sensor
    bool sensorNotRegisteredYet = true;
    for (auto sensor : this->sensorSet) {
      if (sensor == newSensor) {
        sensorNotRegisteredYet = false;
        break;
      }
    }

    if (sensorNotRegisteredYet) {
      this->sensorSet.push_back(newSensor);
      ifSensorRegistered = true;
    }
  }

  return ifSensorRegistered;
}

bool Application::registerUploader(interfaces::IDataUploader* newUploader) {
  bool ifUploaderRegistered = false;

  if (newUploader) {
    // add only new uploader
    bool uploaderNotRegisteredYet = true;
    for (auto uploader : this->uploaderSet) {
      if (uploader == newUploader) {
        uploaderNotRegisteredYet = false;
        break;
      }
    }

    if (uploaderNotRegisteredYet) {
      this->uploaderSet.push_back(newUploader);
      ifUploaderRegistered = true;
    }
  }

  return ifUploaderRegistered;
}

bool Application::registerUi(interfaces::IUi* newUi) {
  bool ifUiRegistered = false;

  if (newUi) {
    this->ui = newUi;
    ifUiRegistered = true;
  }

  return ifUiRegistered;
}

bool Application::registerConfigurator(
    interfaces::IConfigurator* newConfigurator) {
  bool ifConfiguratorRegistered = false;

  if (newConfigurator) {
    this->configurator = newConfigurator;
    ifConfiguratorRegistered = true;
  }

  return ifConfiguratorRegistered;
}

void Application::setInterMeasurementsDelay(uint8_t seconds) {
  this->interMeasurementsDelaySec = seconds;
}

bool Application::setup() {
  bool ifSetupOk = true;

  // welcome message
  this->console.println();
  this->console.println("+------------------------------+");
  this->console.println("| Environmental Data Collector |");
  this->console.println("|           by SP6HFE          |");
  this->console.println("+------------------------------+");
  this->console.println();

  // init all sensors
  for (auto sensor : this->sensorSet) {
    if (sensor->init()) {
      this->console.print("Data source \"");
      this->console.print(sensor->getName());
      this->console.println("\" initialized.");
    } else {
      this->console.print("Could not initialize \"");
      this->console.print(sensor->getName());
      this->console.println("\" sensor. Check wiring and device address!");
    }
  }

  // try to setup upload connections
  bool anyUploaderConnectionWorked = false;
  for (auto uploader : this->uploaderSet) {
    if (uploader->uploadLinkSetup()) {
      anyUploaderConnectionWorked = true;
    }
  }

  if (anyUploaderConnectionWorked) {
    this->console.println("Starting data gathering.");
    this->opMode = OpMode::DATA_GATHERING;
  } else {
    // setup configurator in case of upload connection failure
    this->console.println();
    this->console.println(
        "Error setting up upload data links - starting configurator...");

    if (this->configurator->start()) {
      this->opMode = OpMode::CONFIG;
    } else {
      this->console.println(
          "Fatal error - there is no way to configure the device!");
      this->opMode = OpMode::RESTART;
      ifSetupOk = false;
    }
  }

  return ifSetupOk;
}

void Application::loop(unsigned long loopEnterMillis) {
  switch (this->opMode) {
    case OpMode::DATA_GATHERING:
      // data gathering from all sources
      {
        this->lastDataGatheringMillis = loopEnterMillis;

        if (this->ui) {
          this->ui->setState(interfaces::UiDeviceState::DATA_GATHERING);
        }

        auto status = this->gatherData(loopEnterMillis);
        if (status == StackedExecutionResult::FAILED) {
          if (this->ui) {
            this->ui->setState(interfaces::UiDeviceState::ERROR);
          }
          this->opMode = OpMode::IDLE;
        } else {
          this->opMode = OpMode::DATA_LOGGING;
        }
      }
      break;
    case OpMode::DATA_LOGGING:
      // each collected data to be logged
      this->logData();
      this->opMode = OpMode::DATA_UPLOADING;
      break;
    case OpMode::DATA_UPLOADING:
      // gathered data uploading via all routes
      {
        auto uploadResult = this->uploadData();

        switch (uploadResult) {
          case StackedExecutionResult::FAILED:
            this->console.println(
                "Can't upload data due missing uploading routes.");
            break;
          case StackedExecutionResult::SOME_FAILED:
            this->console.println("Data was not uploaded via all routes.");
            break;
          default:
            break;
        }

        this->opMode = OpMode::IDLE;
      }
      break;
    case OpMode::IDLE:
      // awaiting till next data gathering
      if (loopEnterMillis - this->lastDataGatheringMillis >=
          this->interMeasurementsDelaySec * 1000) {
        this->opMode = OpMode::DATA_GATHERING;
      } else {
        delay(10);
      }
      break;
    case OpMode::CONFIG:
      // configurator handling
      {
        int configuratorExitCode = this->configurator->perform();

        if (configuratorExitCode < 0) {
          this->opMode = OpMode::RESTART;
        } else if (configuratorExitCode == 0) {
          this->opMode = OpMode::DATA_GATHERING;
        }
      }
      break;
    default:
      /* fall though */
    case OpMode::RESTART:
      this->console.println("Restarting...");
      this->system.restart();
      break;
  }
}
