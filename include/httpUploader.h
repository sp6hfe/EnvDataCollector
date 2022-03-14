#pragma once

#include <Stream.h>
#include <cstdint>

#include "IDataUploader.h"
#include "IHttp.h"
#include "IWiFi.h"
#include "helpers.h"

namespace uploaders {

class HttpUploader : public interfaces::IDataUploader {
 private:
  char floatToCharConversionBuffer[8];  // max length is pressure: '1020.57' +
                                        // '\0'

  Stream &console;
  interfaces::IWiFi &wifiCore;
  interfaces::IHttp &httpCore;
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
    this->wifiCore.wifiBegin();

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

    if (!this->wifiCore.wifiConnected()) {
      this->console.print("Connecting to WiFi AP: \"");
      this->console.print(this->wifiSsid);
      this->console.print("\"");
      this->wifiCore.wifiBegin();

      if (!this->wifiCore.wifiConnect(this->wifiSsid.c_str(),
                                      this->wifiPass.c_str(),
                                      this->wifiConnectionTimeoutSec)) {
        this->console.println();
        ifLinkSetup = false;
      } else {
        this->console.println();
        this->console.print("Connected with IP: ");
        this->console.print(this->wifiCore.wifiGetIp());
        this->console.println(".");
      }
    }

    return ifLinkSetup;
  }

  bool upload() override {
    bool ifDataUploaded = false;

    if (this->uploadUrl.length() && this->apiKey.length() &&
        this->token.length() && this->dataToUpload.length() &&
        this->uploadLinkSetup() && this->httpCore.httpBegin(this->uploadUrl)) {
      this->httpCore.httpAddHeader("Content-Type",
                                   "application/x-www-form-urlencoded");
#ifdef DEBUG
      this->console.print(this->dataToUpload);
#endif

      int postResponseCode = this->httpCore.httpSendPost(this->dataToUpload);

#ifdef DEBUG
      this->console.print(" - ");
      this->console.println(postResponseCode);
#endif

      if (postResponseCode == 200) {
        ifDataUploaded = true;
      }

      this->httpCore.httpEnd();
    }

    return ifDataUploaded;
  }

  explicit HttpUploader(Stream &console_, interfaces::IWiFi &wifiCore_,
                        interfaces::IHttp &httpCore_, String name_)
      : console(console_),
        wifiCore(wifiCore_),
        httpCore(httpCore_),
        name(name_){};
};

}  // namespace uploaders