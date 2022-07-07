#pragma once

#include "Hyc3DCommon.h"

#include <string>

class Component {
private:
  const std::string ComponentName;
  STATUS ComponentStatus;

public:
  Component(const std::string &CompName);
  virtual ~Component();

  Component &operator=(const Component &Source) {
    if (this == &Source) {
      return *this;
    }
    const_cast<std::string &>(ComponentName) = Source.ComponentName;
    ComponentStatus = Source.ComponentStatus;
    return *this;
  }

  const std::string &getCompName() const;

  STATUS getCompStatus() const;
  void setCompStatus(STATUS CompStatus);

public:
  virtual bool init() = 0;
  virtual void update(Timer &Timer) = 0;
  virtual void destory() = 0;
};
