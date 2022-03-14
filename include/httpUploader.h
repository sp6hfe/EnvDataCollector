#pragma once

#include <Stream.h>
#include <cstdint>

#include "IDataUploader.h"
#include "IWiFi.h"
#include "helpers.h"

namespace uploaders {

class HttpUploader : public interfaces::IDataUploader {
 private:
  char floatToCharConversionBuffer[8];  // max length is pressure: '1020.57' +
                                        // '\0'

  Stream &console;
  interfaces::IWiFi &wifi;
  String name;
  String uploadUrl;
  String apiKey;
  String token;
  String dataToUpload;

  String wifiSsid = "notConfigured";
  String wifiPass = "noPass";
  uint8_t wifiConnectionTimeoutSec = 10;

 public:
  void begin(const char *wifiSsid_, const char *wifiPass_,
             const uint8_t wifiConnectionTimeoutSec_, const char *uploadUrl_,
             const char *apiKey_, const char *token_) {
    // reset wifi state
    this->wifi.wifiBegin();

    // apply congiguration
    if (wifiSsid_ && wifiPass_ && (wifiConnectionTimeoutSec_ > 0) &&
        uploadUrl_ && apiKey_ && token_) {
      this->wifiSsid = wifiSsid_;
      this->wifiPass = wifiPass_;
      this->wifiConnectionTimeoutSec = wifiConnectionTimeoutSec_;
      this->uploadUrl = uploadUrl_;
      this->apiKey = apiKey_;
      this->token = token_;
    }
  }

  String getName() const override { return this->name; }

  void clearData() override {
    this->dataToUpload = "";
    this->dataToUpload += "api_key=";
    this->dataToUpload += this->apiKey;
    this->dataToUpload += "&token=";
    this->dataToUpload += this->token;
  };

  bool addData(const String name, const float value) override {
    bool ifDataAdded = false;

    if (tools::Helpers::floatToChar(value, floatToCharConversionBuffer)) {
      this->dataToUpload += "&";
      this->dataToUpload += name;
      this->dataToUpload += "=";
      this->dataToUpload += floatToCharConversionBuffer;
      ifDataAdded = true;
    }

    return ifDataAdded;
  }

  bool uploadLinkSetup() override {
    bool ifLinkSetup = true;

    if (!this->wifi.wifiConnected()) {
      this->console.print("Connecting to WiFi AP: \"");
      this->console.print(this->wifiSsid);
      this->console.print("\"");
      this->wifi.wifiBegin();

      if (!this->wifi.wifiConnect(this->wifiSsid.c_str(),
                                  this->wifiPass.c_str(),
                                  this->wifiConnectionTimeoutSec)) {
        this->console.println();
        ifLinkSetup = false;
      } else {
        this->console.println();
        this->console.print("Connected with IP: ");
        this->console.print(this->wifi.wifiGetIp());
        this->console.println(".");
      }
    }

    return ifLinkSetup;
  }

  bool upload() override {
    bool ifDataUploaded = false;

    if (this->uploadUrl.length() && this->apiKey.length() &&
        this->token.length() && this->dataToUpload.length() &&
        this->uploadLinkSetup() && this->wifi.httpBegin(this->uploadUrl)) {
      this->wifi.httpAddHeader("Content-Type",
                               "application/x-www-form-urlencoded");
#ifdef DEBUG
      this->console.print(this->dataToUpload);
#endif

      int postResponseCode = this->wifi.httpSendPost(this->dataToUpload);

#ifdef DEBUG
      this->console.print(" - ");
      this->console.println(postResponseCode);
#endif

      if (postResponseCode == 200) {
        ifDataUploaded = true;
      }

      this->wifi.httpEnd();
    }

    return ifDataUploaded;
  }

  explicit HttpUploader(Stream &console_, interfaces::IWiFi &wifi_,
                        String name_)
      : console(console_), wifi(wifi_), name(name_){};
};

}  // namespace uploaders