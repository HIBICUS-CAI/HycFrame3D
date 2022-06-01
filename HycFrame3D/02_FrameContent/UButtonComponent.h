#pragma once

#include "UiComponent.h"
#include <array>

constexpr auto SELECTED_BTN_SPRITE_NAME = "selected-button-falg";
constexpr auto NULL_BTN = "null-btn";

constexpr UINT BTN_UP = 0;
constexpr UINT BTN_DOWN = 1;
constexpr UINT BTN_LEFT = 2;
constexpr UINT BTN_RIGHT = 3;

class UButtonComponent :public UiComponent
{
public:
    UButtonComponent(std::string&& _compName, class UiObject* _uiOwner);
    UButtonComponent(std::string& _compName, class UiObject* _uiOwner);
    virtual ~UButtonComponent();

    UButtonComponent& operator=(const UButtonComponent& _source)
    {
        if (this == &_source) { return *this; }
        mSurroundBtnObjectNames = _source.mSurroundBtnObjectNames;
        mIsSelected = _source.mIsSelected;
        UiComponent::operator=(_source);
        return *this;
    }

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    void SetUpBtnObjName(std::string _upBtn);
    void SetDownBtnObjName(std::string _downBtn);
    void SetLeftBtnObjName(std::string _leftBtn);
    void SetRightBtnObjName(std::string _rightBtn);

    void SetIsBeingSelected(bool _beingSelected);
    bool IsBeingSelected() const;

    bool IsCursorOnBtn();

    void SelectUpBtn();
    void SelectDownBtn();
    void SelectLeftBtn();
    void SelectRightBtn();

    UButtonComponent* GetUpBtn();
    UButtonComponent* GetDownBtn();
    UButtonComponent* GetLeftBtn();
    UButtonComponent* GetRightBtn();

    static void SetScreenSpaceCursorPos(float _inputX, float _inputY);
    static void SetShouldUseMouse(bool _shouldMouse);

private:
    void SyncDataFromTransform();

private:
    std::array<std::string, 4> mSurroundBtnObjectNames;
    bool mIsSelected;
};
