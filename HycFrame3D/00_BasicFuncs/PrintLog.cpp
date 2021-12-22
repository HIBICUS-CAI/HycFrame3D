#include "PrintLog.h"
#include <assert.h>

// FOR SETTING LOG LEVEL ------------------------------
#define LOG_LEVEL_FOR_SETTING   (LOG_WARNING)
// FOR SETTING LOG LEVEL ------------------------------

int VDebugPrintF(const char* format, va_list argList)
{
    const UINT32 MAX_CHARS = 1024;
    static char s_LogBuffer[MAX_CHARS];

    int charsWritten = vsnprintf(
        s_LogBuffer, MAX_CHARS, format, argList);

    OutputDebugString(s_LogBuffer);

    return charsWritten;
}

int DebugPrintF(int level, const char* format, ...)
{
    static bool invalid_log_level = false;

    if (level >= LOG_LEVEL_FOR_SETTING)
    {
        switch (level)
        {
        case LOG_MESSAGE:
            OutputDebugString("[[[MESSAGE]]] : "); break;
        case LOG_WARNING:
            OutputDebugString("[[[WARNING]]] : "); break;
        case LOG_DEBUG:
            OutputDebugString("[[[DEBUG]]] : "); break;
        case LOG_ERROR:
            OutputDebugString("[[[ERROR]]] : "); break;
        default:
            assert(invalid_log_level); break;
        }

        va_list argList;
        va_start(argList, format);

        int charsWritten = VDebugPrintF(format, argList);

        va_end(argList);

        return charsWritten;
    }
    else
    {
        return -1;
    }
}

int VMyPrintF(const char* format, va_list argList)
{
    return vprintf(format, argList);
}

int MyPrintF(int level, const char* format, ...)
{
    if (level >= LOG_LEVEL_FOR_SETTING)
    {
        va_list argList;
        va_start(argList, format);

        int charsWritten = VMyPrintF(format, argList);

        va_end(argList);

        return charsWritten;
    }
    else
    {
        return -1;
    }
}
