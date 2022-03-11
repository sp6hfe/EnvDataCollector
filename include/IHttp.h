#pragma once

#include <WString.h>

namespace interfaces {
class IHttp {
 public:
  virtual bool httpBegin(const String &url) = 0;
  virtual void httpAddHeader(const String &name, const String &value,
                             bool first = false, bool replace = true) = 0;
  virtual int httpSendPost(const String &payload) = 0;
  virtual void httpEnd() = 0;
};
}  // namespace interfaces