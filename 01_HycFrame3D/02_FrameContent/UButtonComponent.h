#pragma once

#include "UiComponent.h"

#include <array>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#endif // __clang__

constexpr auto SELECTED_BTN_SPRITE_NAME = "selected-button-falg";
constexpr auto NULL_BTN = "null-btn";

constexpr UINT BTN_UP = 0;
constexpr UINT BTN_DOWN = 1;
constexpr UINT BTN_LEFT = 2;
constexpr UINT BTN_RIGHT = 3;

#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

class UButtonComponent : public UiComponent {
private:
  std::array<std::string, 4> SurroundBtnObjectNames;
  bool SelectedFlag;

public:
  UButtonComponent(const std::string &CompName, class UiObject *UiOwner);
  virtual ~UButtonComponent();

  UButtonComponent &operator=(const UButtonComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    SurroundBtnObjectNames = Source.SurroundBtnObjectNames;
    SelectedFlag = Source.SelectedFlag;
    UiComponent::operator=(Source);
    return *this;
  }

public:
  virtual bool init();
  virtual void update(const Timer &Timer);
  virtual void destory();

public:
  void setUpBtnObjName(const std::string &UpBtn);
  void setDownBtnObjName(const std::string &DownBtn);
  void setLeftBtnObjName(const std::string &LeftBtn);
  void setRightBtnObjName(const std::string &RightBtn);

  void setIsBeingSelected(bool BeingSelected);
  bool isBeingSelected() const;

  bool isCursorOnBtn();

  void selectUpBtn();
  void selectDownBtn();
  void selectLeftBtn();
  void selectRightBtn();

  UButtonComponent *getUpBtn();
  UButtonComponent *getDownBtn();
  UButtonComponent *getLeftBtn();
  UButtonComponent *getRightBtn();

  static void setScreenSpaceCursorPos(float InputX, float InputY);
  static void setShouldUseMouse(bool ShouldMouse);

private:
  void syncDataFromTransform();
};
