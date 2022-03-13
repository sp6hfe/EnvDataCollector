#include "application.h"

using namespace application;

bool Application::uploadLinkReady() {
  bool ifLinkActive = true;

  if (!this->wifi.wifiConnected()) {
    this->console.print("Connecting to WiFi AP: \"");
    this->console.print(this->wifiSsid);
    this->console.print("\"");
    this->wifi.wifiBegin();

    if (!this->wifi.wifiConnect(this->wifiSsid.c_str(), this->wifiPass.c_str(),
                                this->wifiConnectionTimeoutSec)) {
      this->console.println();
      ifLinkActive = false;
    } else {
      this->console.println();
      this->console.print("Connected with IP: ");
      this->console.print(this->wifi.wifiGetIp());
      this->console.println(".");
    }
  }

  return ifLinkActive;
}

bool Application::logAndUpload(bool uploadAllowed) {
  bool anyNewMeasurement = false;
  bool logAndUploadResult = false;

  for (auto uploader : this->uploaderSet) {
    uploader->clearData();
  }

  this->console.println();
  this->console.print("New measurements:");

  for (auto sensor : this->sensorSet) {
    if (sensor->newData()) {
      // assumption here is that we want to upload only new measurements and
      // newValue() returns false after first readout with getValue()
      if (uploadAllowed) {
        for (auto uploader : this->uploaderSet) {
          uploader->addData(sensor->getDataId(), sensor->getData());
        }
      }

      this->console.println();
      this->console.print(sensor->getName());
      this->console.print(": ");
      this->console.print(sensor->getData());
      this->console.print("[");
      this->console.print(sensor->getUnit());
      this->console.print("]");
      anyNewMeasurement = true;
    }
  }

  if (!anyNewMeasurement) {
    // nothing to log/upload
    this->console.println(" no new measurements - check log for errors.");
  } else if (uploadAllowed) {
    this->console.println();
    // new logged measurements queued for upload
    auto succeededUploads = 0;
    for (auto uploader : this->uploaderSet) {
      if (uploader->upload()) {
        succeededUploads++;
        this->console.print("(+) Data uploaded");
        logAndUploadResult = true;
      } else {
        this->console.print("(-) Error uploading");
      }
      this->console.print(" via \"");
      this->console.print(uploader->getName());
      this->console.println("\".");
    }
    // success if any data went through
    if (succeededUploads) {
      logAndUploadResult = true;
    }
  } else {
    // new measurements logged
    logAndUploadResult = true;
  }

  return logAndUploadResult;
}

bool Application::registerSensor(interfaces::ISensor *newSensor) {
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

bool Application::registerUploader(interfaces::IDataUploader *newUploader) {
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

bool Application::registerConfigurator(
    interfaces::IConfigurator *newConfigurator) {
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

void Application::setWifiConnectionParams(const char *ssid, const char *pass,
                                          uint8_t timeoutSec) {
  this->wifiSsid = ssid;
  this->wifiPass = pass;
  this->wifiConnectionTimeoutSec = timeoutSec;
}

bool Application::setup() {
  bool ifSetupOk = true;

  // reset wifi state
  this->wifi.wifiBegin();

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
      this->console.print("Sensor \"");
      this->console.print(sensor->getName());
      this->console.println("\" initialized.");
    } else {
      this->console.print("Could not initialize \"");
      this->console.print(sensor->getName());
      this->console.println("\" sensor. Check wiring and device address!");
    }
  }

  // setup upload connection
  if (this->uploadLinkReady()) {
    this->console.println("Starting measurements.");
    this->opMode = OpMode::MEASUREMENTS;
  } else {
    // setup configurator in case of upload connection failure
    this->console.println();
    this->console.println("Data link error - setting up configurator...");

    if (this->configurator->start()) {
      this->opMode = OpMode::CONFIG;
    } else {
      this->console.println("There is no way to configure the device.");
      this->opMode = OpMode::RESTART;
      ifSetupOk = false;
    }
  }

  return ifSetupOk;
}

void Application::loop(unsigned long loopEnterMillis) {
  switch (this->opMode) {
    case OpMode::MEASUREMENTS:
      // deal with WiFi first not to delay upload after measurements
      {
        bool linkReady = this->uploadLinkReady();

        // collect measurements
        for (auto sensor : this->sensorSet) {
          if (!sensor->measure(loopEnterMillis)) {
            this->console.print("Error taking measurements with \"");
            this->console.print(sensor->getName());
            this->console.println("\" sensor.");
          }
        }

        if (!linkReady) {
          this->console.println("Can't upload data due to WiFi link down.");
        }

        if (!this->logAndUpload(linkReady)) {
          this->console.println("Error on data logging/uploading.");
        }

        delay(this->interMeasurementsDelaySec * 1000);
      }
      break;
    case OpMode::CONFIG:
      // handle configurator exit code
      {
        int configuratorExitCode = this->configurator->perform();

        if (configuratorExitCode < 0) {
          this->opMode = OpMode::RESTART;
        } else if (configuratorExitCode == 0) {
          this->opMode = OpMode::MEASUREMENTS;
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
