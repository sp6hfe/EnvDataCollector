#pragma once

#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <Stream.h>

#include "IHttp.h"

namespace wrappers {

class WifiCore : public interfaces::IHttp {
 private:
  Stream &console;
  WiFiClient wifi_client;
  HTTPClient http_client;
  ESP8266WebServer web_server;

 public:
  void restart() { ESP.restart(); }

  void wifiBegin() {
    // cleaning after previous configuration that may be still active
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.persistent(false);
  }

  bool wifiConnect(const char *wifi_ssid, const char *wifi_passphrase,
                   const uint8_t timeout_sec) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(wifi_ssid, wifi_passphrase);

    auto timeout_counter = 0;
    while (WiFi.status() != WL_CONNECTED && timeout_counter < timeout_sec) {
      this->console.print(".");
      delay(1000);
      ++timeout_counter;
    }

    return (WiFi.status() == WL_CONNECTED);
  }

  bool wifiConnected() { return WiFi.isConnected(); }

  IPAddress wifiGetIp() { return WiFi.localIP(); }

  bool apBegin(const char *ssid, const char *psk, int channel, int ssid_hidden,
               int max_connection) {
    return WiFi.softAP(ssid, psk, channel, ssid_hidden, max_connection);
  }

  IPAddress apGetIp() { return WiFi.softAPIP(); }

  void webserverBegin() { this->web_server.begin(); }

  void webserverRegisterPage(const char *uri,
                             std::function<void(void)> callback) {
    this->web_server.on(uri, callback);
  }

  const String &webserverGetArg(const String &name) {
    return this->web_server.arg(name);
  }

  void webserverSend(int code, const char *content_type,
                     const String &content) {
    this->web_server.send(code, content_type, content);
  }

  void webserverPerform() { this->web_server.handleClient(); }

  bool httpBegin(const String &url) override {
    return this->http_client.begin(this->wifi_client, url);
  }

  void httpAddHeader(const String &name, const String &value,
                     bool first = false, bool replace = true) override {
    this->http_client.addHeader(name, value, first, replace);
  }

  int httpSendPost(const String &payload) override {
    return this->http_client.POST(payload);
  }

  void httpEnd() override { this->http_client.end(); }

  explicit WifiCore(Stream &console_, uint16_t web_server_port_)
      : console(console_), web_server(web_server_port_){};
};

}  // namespace wrappers