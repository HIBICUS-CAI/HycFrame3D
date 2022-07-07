#pragma once

#include "Component.h"

class UiComponent : public Component {
private:
  class UiObject *UiOwner;

public:
  UiComponent(const std::string &CompName, class UiObject *UiOwner);
  virtual ~UiComponent();

  UiComponent &operator=(const UiComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    UiOwner = Source.UiOwner;
    Component::operator=(Source);
    return *this;
  }

  class SceneNode &getSceneNode() const;
  class UiObject *getUiOwner() const;

  void resetUiOwner(class UiObject *Owner);

public:
  virtual bool init() = 0;
  virtual void update(Timer &Timer) = 0;
  virtual void destory() = 0;
};
