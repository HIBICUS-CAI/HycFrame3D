#pragma once

#include "UiComponent.h"

using UiInputProcessFuncType = void(*)(class UInputComponent*, Timer&);

class UInputComponent :public UiComponent
{
public:
    UInputComponent(std::string&& _compName, class UiObject* _uiOwner);
    UInputComponent(std::string& _compName, class UiObject* _uiOwner);
    virtual ~UInputComponent();

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    void SetInputFunction(UiInputProcessFuncType _func);
    void ClearInputFunction();

private:
    UiInputProcessFuncType mInputPrecessFunctionPtr;
};
