#pragma once

#ifdef _WIN32
#ifdef MY04WINDOWMANAGER_EXPORTS
#define WINDOWMANAGER_EXPORT __declspec(dllexport)
#else
#define WINDOWMANAGER_EXPORT __declspec(dllimport)
#endif // MY04WINDOWMANAGER_EXPORTS
#else
// shared objects export every symbol with default visibility
#define WINDOWMANAGER_EXPORT
#endif // _WIN32
