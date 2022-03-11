#pragma once

#include <Stream.h>

#include "IDataUploader.h"
#include "IHttp.h"
#include "helpers.h"

namespace uploaders {

class HttpUploader : public interfaces::IDataUploader {
 private:
  char floatToCharConversionBuffer[8];  // max length is pressure: '1020.57' +
                                        // '\0'

  Stream &console;
  interfaces::IHttp &httpCore;
  String name;
  String uploadUrl;
  String apiKey;
  String token;
  String dataToUpload;

 public:
  void begin(const char *uploadUrl_, const char *apiKey_, const char *token_) {
    this->uploadUrl = "";
    this->uploadUrl += uploadUrl_;
    this->apiKey = "";
    this->apiKey += apiKey_;
    this->token = "";
    this->token += token_;
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

  bool upload() override {
    bool ifDataUploaded = false;

    if (this->uploadUrl.length() && this->apiKey.length() &&
        this->token.length() && this->dataToUpload.length() &&
        this->httpCore.httpBegin(this->uploadUrl)) {
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

  explicit HttpUploader(Stream &console_, interfaces::IHttp &httpCore_,
                        String name_)
      : console(console_), httpCore(httpCore_), name(name_){};
};

}  // namespace uploaders