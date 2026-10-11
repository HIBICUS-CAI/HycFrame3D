#include "ID_Common.h"

#include "InputDeviceBase.h"

InputDeviceBase::InputDeviceBase(INPUT_DEVICE_TYPE DeviceType)
    : DeviceType(DeviceType) {}

InputDeviceBase::~InputDeviceBase() {}

INPUT_DEVICE_TYPE InputDeviceBase::getInputDeviceType() { return DeviceType; }
