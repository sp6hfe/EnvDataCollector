#pragma once

#include <WString.h>
#include <functional>

namespace interfaces {

class IWebServer {
 public:
  virtual void webserverBegin() = 0;
  virtual void webserverRegisterPage(const char *uri,
                                     std::function<void(void)> callback) = 0;
  virtual const String &webserverGetArg(const String &name) = 0;
  virtual void webserverSend(int code, const char *content_type,
                             const String &content) = 0;
  virtual void webserverPerform() = 0;
};

}  // namespace interfaces
