#pragma once

#include <cstdint>
#include "IPAddress.h"

namespace interfaces {

class IWiFi {
 public:
  virtual void wifiBegin() = 0;
  virtual bool wifiConnect(const char *wifi_ssid, const char *wifi_passphrase,
                           const uint8_t timeout_sec) = 0;
  virtual bool wifiConnected() = 0;
  virtual IPAddress wifiGetIp() = 0;

  virtual bool apBegin(const char *ssid, const char *psk, int channel,
                       int ssid_hidden, int max_connection) = 0;
  virtual IPAddress apGetIp() = 0;
};

}  // namespace interfaces
