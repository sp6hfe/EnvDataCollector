#include "application.h"

#include "config.h"

using namespace application;

void Application::root_web_page() {
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

void Application::config_web_page() {
  String new_ssid = this->wifiCore.webserverGetArg("ssid");
  String new_pass = this->wifiCore.webserverGetArg("pass");
  String content;
  int web_server_status_code = 200;

  if (new_ssid.length() > 0 && new_pass.length() > 0) {
    this->console.println("Received data:");
    this->console.print("ssid: ");
    this->console.println(new_ssid);
    this->console.print("pass: ");
    this->console.println(new_pass);
    content = "{\"Success\":\"Data received.\"}";
  } else {
    this->console.println("Received data not usable. Sending 404.");
    content = "{\"Error\":\"404 not found\"}";
    web_server_status_code = 404;
  }
  this->wifiCore.webserverSend(web_server_status_code, "application/json",
                               content);
}

void Application::restart_web_page() {
  String content = "{\"Success\":\"Self reset. Bye!\"}";
  this->console.println("Self reset. Bye!");
  this->wifiCore.webserverSend(200, "application/json", content);

  // delay to allow web server push the data before restarting
  delay(1500);
  this->wifiCore.restart();
}

void Application::configure_web_server() {
  this->wifiCore.webserverRegisterPage(
      "/", [this]() -> void { this->root_web_page(); });
  this->wifiCore.webserverRegisterPage(
      "/config", [this]() -> void { this->config_web_page(); });
  this->wifiCore.webserverRegisterPage(
      "/restart", [this]() -> void { this->restart_web_page(); });
}

void Application::log_measurements() {
  this->console.print("Measurements:");
  for (auto sensor : this->sensorSet) {
    this->console.print(" ");
    this->console.print(sensor->getValue());
    this->console.print("[");
    this->console.print(sensor->getUnit());
    this->console.print("]");
  }
  this->console.println();
}

bool Application::upload_link_ready(const char *wifi_ssid,
                                    const char *wifi_passphrase,
                                    const uint8_t timeout_sec) {
  bool if_link_active = true;

  if (!this->wifiCore.wifiConnected()) {
    this->console.print("Connecting to WiFi AP: \"");
    this->console.print(wifi_ssid);
    this->console.print("\"");
    this->wifiCore.wifiBegin();

    if (!this->wifiCore.wifiConnect(wifi_ssid, wifi_passphrase, timeout_sec)) {
      this->console.println();
      if_link_active = false;
    } else {
      this->console.println();
      this->console.print("Connected with IP: ");
      this->console.print(this->wifiCore.wifiGetIp());
      this->console.println(".");
    }
  }

  return if_link_active;
}

bool Application::upload_data() {
  this->dataUploader.clearData();

  for (auto sensor : this->sensorSet) {
    this->dataUploader.addData(sensor->getName(), sensor->getValue());
  }

  return this->dataUploader.upload();
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
  bool if_setup_ok = true;

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
  if (this->upload_link_ready(config::ssid, config::pass,
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
      if_setup_ok = false;
    }
  }

  return if_setup_ok;
}

void Application::loop(unsigned long loop_enter_millis) {
  switch (this->opMode) {
    case OpMode::MEASUREMENTS:
      // deal with WiFi first not to delay upload after measurements
      {
        bool link_ready = this->upload_link_ready(
            config::ssid, config::pass, config::wifi_connect_timeout_sec);

        // collect measurements
        for (auto sensor : this->sensorSet) {
          if (!sensor->measure(loop_enter_millis)) {
            this->console.print("Error taking measurements with \"");
            this->console.print(sensor->getName());
            this->console.println("\" sensor.");
          }
        }

        // for BME280 measurement of one parameter give all readouts
        this->log_measurements();
        if (!link_ready) {
          this->console.println("Can't upload data due to WiFi link down.");
        } else {
          if (!this->upload_data()) {
            this->console.println("Error on data uploading.");
          }
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
