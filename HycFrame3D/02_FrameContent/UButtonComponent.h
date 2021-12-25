#pragma once

#include "UiComponent.h"
#include <array>

constexpr auto NULL_BTN = "null-btn";

class UButtonComponent :public UiComponent
{
public:
    UButtonComponent(std::string&& _compName, class UiObject& _uiOwner);
    UButtonComponent(std::string& _compName, class UiObject& _uiOwner);
    virtual ~UButtonComponent();

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    void SetIsBeingSelected(bool _beingSelected);
    bool IsBeingSelected() const;

    void SelectUpBtn();
    void SelectDownBtn();
    void SelectLeftBtn();
    void SelectRightBtn();

    UButtonComponent* GetUpBtn();
    UButtonComponent* GetDownBtn();
    UButtonComponent* GetLeftBtn();
    UButtonComponent* GetRightBtn();

private:
    std::array<std::string, 4> mSurroundBtnCompNames;
    bool mIsSelected;
};
