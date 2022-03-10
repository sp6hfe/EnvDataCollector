#include "application.h"

#include "config.h"

using namespace application;

void Application::webPageRoot() {
  this->console.println("Client has accessed main page.");
  IPAddress ip = this->wifiCore.apGetIp();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) +
                 '.' + String(ip[3]);
  String content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
  content += ipStr;
  content += "<p>";
  //   this->webContent += st;
  content +=
      "</p><form method='get' action='setting'><label>SSID: "
      "</label><input name='ssid' length=32><input name='pass' "
      "length=64><input type='submit'></form>";
  content += "</html>";
  this->wifiCore.webserverSend(200, "text/html", content);
}

void Application::webPageConfig() {
  String newSsid = this->wifiCore.webserverGetArg("ssid");
  String newPassphrase = this->wifiCore.webserverGetArg("pass");
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
  this->wifiCore.webserverSend(webserverStatusCode, "application/json",
                               content);
}

void Application::webPageRestart() {
  String content = "{\"Success\":\"Self reset. Bye!\"}";
  this->console.println("Self reset. Bye!");
  this->wifiCore.webserverSend(200, "application/json", content);

  // delay to allow web server push the data before restarting
  delay(1500);
  this->wifiCore.restart();
}

void Application::webserverConfig() {
  this->wifiCore.webserverRegisterPage(
      "/", [this]() -> void { this->webPageRoot(); });
  this->wifiCore.webserverRegisterPage(
      "/config", [this]() -> void { this->webPageConfig(); });
  this->wifiCore.webserverRegisterPage(
      "/restart", [this]() -> void { this->webPageRestart(); });
}

bool Application::uploadLinkReady(const char *wifiSsid,
                                  const char *wifiPassphrase,
                                  const uint8_t timeoutSec) {
  bool ifLinkActive = true;

  if (!this->wifiCore.wifiConnected()) {
    this->console.print("Connecting to WiFi AP: \"");
    this->console.print(wifiSsid);
    this->console.print("\"");
    this->wifiCore.wifiBegin();

    if (!this->wifiCore.wifiConnect(wifiSsid, wifiPassphrase, timeoutSec)) {
      this->console.println();
      ifLinkActive = false;
    } else {
      this->console.println();
      this->console.print("Connected with IP: ");
      this->console.print(this->wifiCore.wifiGetIp());
      this->console.println(".");
    }
  }

  return ifLinkActive;
}

bool Application::logAndUpload(bool uploadAllowed) {
  bool anyNewMeasurement = false;
  bool logAndUploadResult = false;

  this->dataUploader.clearData();
  this->console.print("Measurements:");

  for (auto sensor : this->sensorSet) {
    if (sensor->newValue()) {
      // assumption here is that we want to upload only new measurements and
      // newValue() returns false after first readout with getValue()
      if (uploadAllowed) {
        this->dataUploader.addData(sensor->getName(), sensor->getValue());
      }

      this->console.print(" ");
      this->console.print(sensor->getValue());
      this->console.print("[");
      this->console.print(sensor->getUnit());
      this->console.print("]");
      anyNewMeasurement = true;
    }
  }
  this->console.print(" ");

  if (!anyNewMeasurement) {
    // nothing to log/upload
    this->console.print("no new measurements - check log for errors");
  } else if (uploadAllowed) {
    // new logged measurements queued for upload
    if (this->dataUploader.upload()) {
      this->console.print(" (+)");
      logAndUploadResult = true;
    } else {
      this->console.print(" (-)");
    }
  } else {
    // new measurements logged
    logAndUploadResult = true;
  }
  this->console.println(".");

  return logAndUploadResult;
}

bool Application::registerSensor(interfaces::ISensor *sensor) {
  bool ifSensorRegistered = false;

  if (sensor) {
    // add only new sensor
    bool sensorNotRegisteredYet = true;
    for (std::vector<interfaces::ISensor *>::iterator it =
             this->sensorSet.begin();
         it != this->sensorSet.end(); it++) {
      if (*it == sensor) {
        sensorNotRegisteredYet = false;
        break;
      }
    }

    if (sensorNotRegisteredYet) {
      this->sensorSet.push_back(sensor);
      ifSensorRegistered = true;
    }
  }

  return ifSensorRegistered;
}

bool Application::setup() {
  bool ifSetupOk = true;

  // reset wifi state
  this->wifiCore.wifiBegin();

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

  // init data uploader
  this->dataUploader.begin(config::upload_path, config::api_key, config::token);

  // connect to wifi by default
  if (this->uploadLinkReady(config::ssid, config::pass,
                            config::wifi_connect_timeout_sec)) {
    this->console.println("Starting measurements.");
    this->opMode = OpMode::MEASUREMENTS;
  } else {
    // setup AP for configuration in case of connection error
    this->console.println();
    this->console.println("WiFi connection timeout - starting AP.");
    // TODO: get rid of magic numbers
    if (this->wifiCore.apBegin(config::ap_ssid, config::ap_pass, 5, 0, 2)) {
      this->console.println("");
      this->console.print("AP IP: ");
      this->console.println(this->wifiCore.apGetIp());
      this->console.println("Starting web server.");
      //   configure_web_server();
      this->wifiCore.webserverBegin();
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
        bool linkReady = this->uploadLinkReady(
            config::ssid, config::pass, config::wifi_connect_timeout_sec);

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

        delay(config::inter_measurements_delay_sec * 1000);
      }
      break;
    case OpMode::CONFIG:
      this->wifiCore.webserverPerform();
      break;
    default:
      /* fall though */
    case OpMode::RESTART:
      this->console.println("Restarting...");
      this->wifiCore.restart();
      break;
  }
}
