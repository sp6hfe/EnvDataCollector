#include "application.h"
#include "config.h"
#include "helpers.h"

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
  this->console.print(this->sensorTemperature.getValue());
  this->console.print(" [*C], ");
  this->console.print(this->sensorHumidity.getValue());
  this->console.print(" [%], ");
  this->console.print(this->sensorPressureRaw.getValue());
  this->console.print(" [hPa]");
}

bool Application::upload_data() {
  static constexpr int FRACT_SIZE = 2;
  bool if_uploaded = false;
  char conversion_buffer[8];  // max length is pressure: '1020.57' + '\0'

  if (this->wifiCore.httpBegin(config::upload_path)) {
    this->wifiCore.httpAddHeader("Content-Type",
                                 "application/x-www-form-urlencoded");

    String data_to_upload = "api_key=";
    data_to_upload += config::api_key;
    data_to_upload += "&token=";
    data_to_upload += config::token;

    if (Helpers::floatToChar(this->sensorTemperature.getValue(), FRACT_SIZE,
                             conversion_buffer)) {
      data_to_upload += "&";
      data_to_upload += this->sensorTemperature.getName();
      data_to_upload += "=";
      data_to_upload += conversion_buffer;
    }

    if (Helpers::floatToChar(this->sensorHumidity.getValue(), FRACT_SIZE,
                             conversion_buffer)) {
      data_to_upload += "&";
      data_to_upload += this->sensorHumidity.getName();
      data_to_upload += "=";
      data_to_upload += conversion_buffer;
    }

    if (Helpers::floatToChar(this->sensorPressureRaw.getValue(), FRACT_SIZE,
                             conversion_buffer)) {
      data_to_upload += "&";
      data_to_upload += this->sensorPressureRaw.getName();
      data_to_upload += "=";
      data_to_upload += conversion_buffer;
    }

#ifdef DEBUG
    this->console.println();
    this->console.print(data_to_upload);
#endif

    int post_response = this->wifiCore.httpSendPost(data_to_upload);
    if (post_response == 200) {
      if_uploaded = true;
    }
#ifdef DEBUG
    else {
      this->console.print(" ");
      this->console.print(post_response);
      this->console.print(" ");
    }
#endif

    this->wifiCore.httpEnd();
  }

  return if_uploaded;
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
  if (this->sensorTemperature.init()) {
    this->console.println("Temperature sensor initialized.");
  } else {
    this->console.println(
        "Could not initialize temperature sansor. Check wiring and device "
        "address!");
  }

  if (this->sensorHumidity.init()) {
    this->console.println("Humidity sensor initialized.");
  } else {
    this->console.println(
        "Could not initialize humidity sansor. Check wiring and device "
        "address!");
  }

  if (this->sensorPressureRaw.init()) {
    this->console.println("Pressure sensor initialized.");
  } else {
    this->console.println(
        "Could not initialize pressure sansor. Check wiring and device "
        "address!");
  }

  // connect to wifi by default
  this->console.print("Connecting to WiFi AP: \"");
  this->console.print(config::ssid);
  this->console.print("\"");
  if (this->wifiCore.wifiConnect(config::ssid, config::pass,
                                 config::wifi_connect_timeout_sec)) {
    this->console.println();
    this->console.print("Connected with IP: ");
    this->console.println(this->wifiCore.wifiGetIp());
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

void Application::loop() {
  switch (this->opMode) {
    case OpMode::MEASUREMENTS:
      if (this->sensorTemperature.measure()) {
        this->log_measurements();
        if (!this->upload_data()) {
          this->console.println("Error on data uploading.");
        }
      } else {
        this->console.println("Error on taking measurements.");
      }
      delay(config::inter_measurements_delay_sec * 1000);
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
