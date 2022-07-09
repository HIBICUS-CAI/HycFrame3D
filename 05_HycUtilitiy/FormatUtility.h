#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-once-outside-header"
#endif // __clang__
#pragma once
#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

#include "HycType.h"

#include <spdlog\fmt\fmt.h>

#include <string>

namespace hyc {
namespace str {

template <typename... T>
using FormatString = fmt::format_string<T...>;

using FormatArgs = fmt::dynamic_format_arg_store<fmt::format_context>;

template <typename... T>
std::string sFormat(FormatString<T...> Fmt, T &&...Args) {
  return fmt::format(Fmt, Args...);
}

std::string vFormat(const std::string &FormatTemplate, const FormatArgs &Args) {
  return fmt::vformat(FormatTemplate, Args);
}

template <typename T>
std::string toString(const T &Value) {
  return fmt::to_string(Value);
}

} // namespace str
} // namespace hyc
