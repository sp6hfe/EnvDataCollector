#pragma once

#include <WString.h>
#include <cstdint>
#include "IPAddress.h"

namespace interfaces {

class IWiFi {
 public:
  /* WiFi general */
  virtual void wifiBegin() = 0;
  virtual bool wifiConnect(const char *wifi_ssid, const char *wifi_passphrase,
                           const uint8_t timeout_sec) = 0;
  virtual bool wifiConnected() = 0;
  virtual IPAddress wifiGetIp() = 0;

  /* access point */
  virtual bool apBegin(const char *ssid, const char *psk, int channel,
                       int ssid_hidden, int max_connection) = 0;
  virtual IPAddress apGetIp() = 0;

  /* HTTP protocol */
  virtual bool httpBegin(const String &url) = 0;
  virtual void httpAddHeader(const String &name, const String &value,
                             bool first = false, bool replace = true) = 0;
  virtual int httpSendPost(const String &payload) = 0;
  virtual void httpEnd() = 0;
};

}  // namespace interfaces
