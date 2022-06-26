#pragma once

namespace hyc
{
    using int8 = signed char;
    using int16 = short;
    using int32 = int;
    using int64 = long long;

    using uint8 = unsigned char;
    using uint16 = unsigned short;
    using uint32 = unsigned int;
    using uint64 = unsigned long long;
    using uint = uint32;

    using cstring = const char*;

    constexpr auto INT8_MIN_VALUE = -127I8 - 1;
    constexpr auto INT16_MIN_VALUE = -32767I16 - 1;
    constexpr auto INT32_MIN_VALUE = -2147483647I32 - 1;
    constexpr auto INT64_MIN_VALUE = -9223372036854775807I64 - 1;
    constexpr auto INT8_MAX_VALUE = 127I8;
    constexpr auto INT16_MAX_VALUE = 32767I16;
    constexpr auto INT32_MAX_VALUE = 2147483647I32;
    constexpr auto INT64_MAX_VALUE = 9223372036854775807I64;
    constexpr auto UINT8_MAX_VALUE = 0xFFUI8;
    constexpr auto UINT16_MAX_VALUE = 0xFFFFUI16;
    constexpr auto UINT32_MAX_VALUE = 0xFFFFFFFFUI32;
    constexpr auto UINT64_MAX_VALUE = 0xFFFFFFFFFFFFFFFFUI64;
}
