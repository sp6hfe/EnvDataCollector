#pragma once

#include <Stream.h>
#include <WString.h>

#include "IConfigurator.h"
#include "IWebServer.h"
#include "IWiFi.h"

namespace configurators {

class WebConfigurator : public interfaces::IConfigurator {
 private:
  Stream &console;
  interfaces::IWiFi &wifi;
  interfaces::IWebServer &webServer;

  String apSsid = "EnvDataCollector";
  String apPass = "noPass";
  bool deviceResetRequested = false;

  void webPageRoot() {
    this->console.println();
    this->console.println("Client has accessed main page.");
    IPAddress ip = this->wifi.apGetIp();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) +
                   '.' + String(ip[3]);
    String content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
    content += ipStr;
    content +=
        "<form method='get' action='config'><label>SSID: "
        "</label><input name='ssid' length=32><input name='pass' "
        "length=64><input type='submit'></form>";
    content += "</html>";
    this->webServer.webserverSend(200, "text/html", content);
  }

  void webPageConfig() {
    String newSsid = this->webServer.webserverGetArg("ssid");
    String newPassphrase = this->webServer.webserverGetArg("pass");
    String content;
    int webserverStatusCode = 200;

    if (newSsid.length() > 0 && newPassphrase.length() > 0) {
      this->console.println();
      this->console.println("Received data:");
      this->console.print("ssid: ");
      this->console.println(newSsid);
      this->console.print("pass: ");
      this->console.println(newPassphrase);
      content = "{\"Success\":\"Data received.\"}";
    } else {
      this->console.println();
      this->console.println("Received data not usable. Sending 404.");
      content = "{\"Error\":\"404 not found\"}";
      webserverStatusCode = 404;
    }
    this->webServer.webserverSend(webserverStatusCode, "application/json",
                                  content);
  }

  void webPageRestart() {
    String content = "{\"Success\":\"Self reset. Bye!\"}";
    this->console.println();
    this->console.println("Self reset. Bye!");
    this->webServer.webserverSend(200, "application/json", content);

    // delay to allow web server push the data before restarting
    delay(1500);
    this->deviceResetRequested = true;
  }

 public:
  void begin(const char *apSsid, const char *apPass) {
    if (apSsid && apPass) {
      this->apSsid = apSsid;
      this->apPass = apPass;

      this->webServer.webserverRegisterPage(
          "/", [this]() -> void { this->webPageRoot(); });
      this->webServer.webserverRegisterPage(
          "/config", [this]() -> void { this->webPageConfig(); });
      this->webServer.webserverRegisterPage(
          "/restart", [this]() -> void { this->webPageRestart(); });
    }
  }

  bool start() override {
    bool ifStarted = false;

    this->console.println();
    this->console.print("WiFi configurator - starting \"");
    this->console.print(this->apSsid);
    this->console.println("\" AP...");
    // TODO: get rid of magic numbers
    if (this->wifi.apBegin(this->apSsid.c_str(), this->apPass.c_str(), 5, 0,
                           2)) {
      this->console.println("Done.");

      this->console.println("Starting web server...");
      this->webServer.webserverBegin();
      this->console.println("Done.");

      this->console.print("Configurator's IP: ");
      this->console.print(this->wifi.apGetIp());
      this->console.print(".");

      ifStarted = true;
    } else {
      this->console.println("Failed.");
    }

    return ifStarted;
  }

  int perform() override {
    if (this->deviceResetRequested) {
      this->deviceResetRequested = false;
      return -1;
    }

    this->webServer.webserverPerform();
    return 1;
  }

  WebConfigurator(Stream &console_, interfaces::IWiFi &wifi_,
                  interfaces::IWebServer &webServer_)
      : console(console_), wifi(wifi_), webServer(webServer_) {}
};

}  // namespace configurators
