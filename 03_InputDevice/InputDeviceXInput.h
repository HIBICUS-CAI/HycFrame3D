#pragma once
#include "InputDeviceBase.h"
#include <map>

class InputDeviceXInput :
    public InputDeviceBase
{
public:
    InputDeviceXInput(DWORD xiDeviceHandle);

    ~InputDeviceXInput();

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
    XINPUT_STATE* mDeviceStatus;
    XINPUT_STATE* mDeviceStatusBeforeThisPoll;

    std::map<UINT, WORD> mXIKeyCodeToNorm;
};

