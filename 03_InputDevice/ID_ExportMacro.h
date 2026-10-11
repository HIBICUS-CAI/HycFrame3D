#pragma once

#ifdef _WIN32
#ifdef MY03INPUTDEVICE_EXPORTS
#define INPUTDEVICE_EXPORT __declspec(dllexport)
#else
#define INPUTDEVICE_EXPORT __declspec(dllimport)
#endif // MY03INPUTDEVICE_EXPORTS
#else
// shared objects export every symbol with default visibility
#define INPUTDEVICE_EXPORT
#endif // _WIN32
