#pragma once
#include "InputDeviceBase.h"

class InputDeviceDirectInput :
    public InputDeviceBase
{
public:
    InputDeviceDirectInput(INPUT_DEVICE_TYPE deviceType);

    ~InputDeviceDirectInput();

    virtual INPUT_TYPE getInputType();

    virtual HRESULT PollDeviceStatus();

    virtual const LPVOID getDeviceStatus();

    virtual const bool isKeyBeingPushed(UINT keyCode);
    virtual const bool hasKeyPushedInLastFrame(UINT keyCode);
    virtual const LONG getXPositionOffset();
    virtual const LONG getYPositionOffset();
    virtual const LONG getZPositionOffset();
    virtual const LONG getXRotationOffset();
    virtual const LONG getYRotationOffset();
    virtual const LONG getZRotationOffset();

private:
    LPVOID mDeviceStatus;
    WORD mDeviceStatusSize;
    bool mButtonsStatusBeforeThisPoll[GP_UPLEFTDIRBTN + 0x01];
};

