#pragma once

#include <Stream.h>

#include "IDataUploader.h"
#include "IHttp.h"
#include "helpers.h"

namespace wrappers {

class HttpUploader : public interfaces::IDataUploader {
 private:
  char floatToCharConversionBuffer[8];  // max length is pressure: '1020.57' +
                                        // '\0'

  Stream &console;
  interfaces::IHttp &httpCore;
  String uploadUrl;
  String apiKey;
  String sensorToken;
  String dataToUpload;

 public:
  void begin(const char *uploadUrl_, const char *apiKey_,
             const char *sensorToken_) {
    this->uploadUrl = "";
    this->uploadUrl += uploadUrl_;
    this->apiKey = "";
    this->apiKey += apiKey_;
    this->sensorToken = "";
    this->sensorToken += sensorToken_;
  }

  void clearData() override {
    this->dataToUpload = "";
    this->dataToUpload += "api_key=";
    this->dataToUpload += this->apiKey;
    this->dataToUpload += "&token=";
    this->dataToUpload += this->sensorToken;
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

  bool upload() override {
    bool ifDataUploaded = false;

    if (this->uploadUrl.length() && this->apiKey.length() &&
        this->sensorToken.length() && this->dataToUpload.length() &&
        this->httpCore.httpBegin(this->uploadUrl)) {
      this->httpCore.httpAddHeader("Content-Type",
                                   "application/x-www-form-urlencoded");
#ifdef DEBUG
      this->console.print(this->dataToUpload);
#endif

      int postResponseCode = this->httpCore.httpSendPost(this->dataToUpload);
      if (postResponseCode == 200) {
        ifDataUploaded = true;
      }
#ifdef DEBUG
      else {
        this->console.print(" ");
        this->console.print(postResponseCode);
      }

      this->console.println();
#endif

      this->httpCore.httpEnd();
    }

    return ifDataUploaded;
  }

  explicit HttpUploader(Stream &console_, interfaces::IHttp &httpCore_)
      : console(console_), httpCore(httpCore_){};
};

}  // namespace wrappers