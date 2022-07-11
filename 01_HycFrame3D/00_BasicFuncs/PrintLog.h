#pragma once

#include "TcpLoggerConnection.h"

#include <FormatUtility.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#endif // __clang__

constexpr auto LOG_DEBUG = (0);
constexpr auto LOG_MESSAGE = (1);
constexpr auto LOG_WARNING = (2);
constexpr auto LOG_ERROR = (3);

#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

//#define P_LOG1 debugPrintF
//
// int debugPrintF(int Level, const char *Format, ...);
//
// int myPrintF(int Level, const char *Format, ...);

#define P_LOG(LOG_LEVEL, ...)                                                  \
  TcpLoggerConnection::insertNewLogMessage(LOG_LEVEL,                          \
                                           hyc::str::sFormat(__VA_ARGS__))
