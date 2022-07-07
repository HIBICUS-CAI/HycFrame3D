#include "ID_Common.h"

#include "InputDeviceBase.h"

InputDeviceBase::InputDeviceBase(INPUT_DEVICE_TYPE deviceType,
                                 DWORD xiDeviceHandle)
    : DIDeviceHandle(nullptr), XIDeviceHandle(xiDeviceHandle),
      DeviceType(deviceType) {}

InputDeviceBase::~InputDeviceBase() {}

LPDIRECTINPUTDEVICE8 InputDeviceBase::getDIDeviceHandle() {
  return DIDeviceHandle;
}

DWORD InputDeviceBase::getXIDeviceHandle() { return XIDeviceHandle; }

INPUT_DEVICE_TYPE InputDeviceBase::getInputDeviceType() { return DeviceType; }
