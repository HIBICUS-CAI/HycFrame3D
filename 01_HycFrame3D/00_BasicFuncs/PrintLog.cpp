#include "PrintLog.h"

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN (1)
#endif // !WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
// there is no debugger output channel outside windows, use stderr instead
static void OutputDebugString(const char *Str) { fputs(Str, stderr); }
#endif // _WIN32

// FOR SETTING LOG LEVEL ------------------------------
#define LOG_LEVEL_FOR_SETTING (LOG_DEBUG)
// FOR SETTING LOG LEVEL ------------------------------

int vDebugPrintF(const char *Format, va_list ArgList) {
  const uint32_t MAX_CHARS = 1024;
  static char LogBuffer[MAX_CHARS];

  int CharsWritten = vsnprintf(LogBuffer, MAX_CHARS, Format, ArgList);

  OutputDebugString(LogBuffer);

  return CharsWritten;
}

int debugPrintF(int Level, const char *Format, ...) {
  if (Level >= LOG_LEVEL_FOR_SETTING) {
    switch (Level) {
    case LOG_MESSAGE:
      OutputDebugString("[[[MESSAGE]]] : ");
      break;
    case LOG_WARNING:
      OutputDebugString("[[[WARNING]]] : ");
      break;
    case LOG_DEBUG:
      OutputDebugString("[[[DEBUG]]] : ");
      break;
    case LOG_ERROR:
      OutputDebugString("[[[ERROR]]] : ");
      break;
    default:
      assert(false && "invalid log level");
      break;
    }

    va_list ArgList = {};
    va_start(ArgList, Format);

    int CharsWritten = vDebugPrintF(Format, ArgList);

    va_end(ArgList);

    return CharsWritten;
  } else {
    return -1;
  }
}

int vMyPrintF(const char *Format, va_list ArgList) {
  return vprintf(Format, ArgList);
}

int myPrintF(int Level, const char *Format, ...) {
  if (Level >= LOG_LEVEL_FOR_SETTING) {
    va_list ArgList = {};
    va_start(ArgList, Format);

    int CharsWritten = vMyPrintF(Format, ArgList);

    va_end(ArgList);

    return CharsWritten;
  } else {
    return -1;
  }
}
