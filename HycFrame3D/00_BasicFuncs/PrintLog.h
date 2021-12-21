#pragma once

#include <stdio.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN (1)
#endif // !WIN32_LEAN_AND_MEAN

#include <Windows.h>

#define LOG_MESSAGE             (0)
#define LOG_WARNING             (1)
#define LOG_DEBUG               (2)
#define LOG_ERROR               (3)

#define P_LOG DebugPrintF

int DebugPrintF(int level, const char* format, ...);

int MyPrintF(int level, const char* format, ...);

