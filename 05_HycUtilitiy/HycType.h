#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-once-outside-header"
#endif // __clang__
#pragma once
#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

#include <limits>

namespace hyc {

using int8 = signed char;
using int16 = short;
using int32 = int;
using int64 = long long;

using uint8 = unsigned char;
using uint16 = unsigned short;
using uint32 = unsigned int;
using uint64 = unsigned long long;
using uint = uint32;

using cstring = const char *;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#endif // __clang__

constexpr auto INT8_MIN_VALUE = std::numeric_limits<int8>::min();
constexpr auto INT16_MIN_VALUE = std::numeric_limits<int16>::min();
constexpr auto INT32_MIN_VALUE = std::numeric_limits<int32>::min();
constexpr auto INT64_MIN_VALUE = std::numeric_limits<int64>::min();
constexpr auto INT8_MAX_VALUE = std::numeric_limits<int8>::max();
constexpr auto INT16_MAX_VALUE = std::numeric_limits<int16>::max();
constexpr auto INT32_MAX_VALUE = std::numeric_limits<int32>::max();
constexpr auto INT64_MAX_VALUE = std::numeric_limits<int64>::max();
constexpr auto UINT8_MAX_VALUE = std::numeric_limits<uint8>::max();
constexpr auto UINT16_MAX_VALUE = std::numeric_limits<uint16>::max();
constexpr auto UINT32_MAX_VALUE = std::numeric_limits<uint32>::max();
constexpr auto UINT64_MAX_VALUE = std::numeric_limits<uint64>::max();

#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

} // namespace hyc
