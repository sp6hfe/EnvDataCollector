#include "application.h"

#include "config.h"

using namespace application;

void Application::webPageRoot() {
  this->console.println("Client has accessed main page.");
  IPAddress ip = this->wifi.apGetIp();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) +
                 '.' + String(ip[3]);
  String content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
  content += ipStr;
  content += "<p>";
  //   this->webContent += st;
  content +=
      "</p><form method='get' action='config'><label>SSID: "
      "</label><input name='ssid' length=32><input name='pass' "
      "length=64><input type='submit'></form>";
  content += "</html>";
  this->webServer.webserverSend(200, "text/html", content);
}

void Application::webPageConfig() {
  String newSsid = this->webServer.webserverGetArg("ssid");
  String newPassphrase = this->webServer.webserverGetArg("pass");
  String content;
  int webserverStatusCode = 200;

  if (newSsid.length() > 0 && newPassphrase.length() > 0) {
    this->console.println("Received data:");
    this->console.print("ssid: ");
    this->console.println(newSsid);
    this->console.print("pass: ");
    this->console.println(newPassphrase);
    content = "{\"Success\":\"Data received.\"}";
  } else {
    this->console.println("Received data not usable. Sending 404.");
    content = "{\"Error\":\"404 not found\"}";
    webserverStatusCode = 404;
  }
  this->webServer.webserverSend(webserverStatusCode, "application/json",
                                content);
}

void Application::webPageRestart() {
  String content = "{\"Success\":\"Self reset. Bye!\"}";
  this->console.println("Self reset. Bye!");
  this->webServer.webserverSend(200, "application/json", content);

  // delay to allow web server push the data before restarting
  delay(1500);
  this->system.restart();
}

void Application::webserverConfig() {
  this->webServer.webserverRegisterPage(
      "/", [this]() -> void { this->webPageRoot(); });
  this->webServer.webserverRegisterPage(
      "/config", [this]() -> void { this->webPageConfig(); });
  this->webServer.webserverRegisterPage(
      "/restart", [this]() -> void { this->webPageRestart(); });
}

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
  this->console.println("**********");
  this->console.println("Environmental Data Collector by SP6HFE");
  this->console.println("**********");
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

  // connect to wifi by default
  if (this->uploadLinkReady()) {
    this->console.println("Starting measurements.");
    this->opMode = OpMode::MEASUREMENTS;
  } else {
    // setup AP for configuration in case of connection error
    this->console.println();
    this->console.println("WiFi connection timeout - starting AP.");
    // TODO: get rid of magic numbers
    if (this->wifi.apBegin(config::ap_ssid, config::ap_pass, 5, 0, 2)) {
      this->console.println("");
      this->console.print("AP IP: ");
      this->console.println(this->wifi.apGetIp());
      this->console.println("Starting web server.");
      this->webserverConfig();
      this->webServer.webserverBegin();
      this->console.println("Done.");
      this->opMode = OpMode::CONFIG;
    } else {
      this->console.println(
          "AP setup failed. There is no way to communicate wirelessly.");
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
      this->webServer.webserverPerform();
      break;
    default:
      /* fall though */
    case OpMode::RESTART:
      this->console.println("Restarting...");
      this->system.restart();
      break;
  }
}
