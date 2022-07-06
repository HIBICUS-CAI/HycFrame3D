#pragma once

#define LOG_MESSAGE (0)
#define LOG_WARNING (1)
#define LOG_DEBUG (2)
#define LOG_ERROR (3)

#define P_LOG debugPrintF

int
debugPrintF(int Level, const char *Format, ...);

int
myPrintF(int Level, const char *Format, ...);
