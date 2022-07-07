#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-once-outside-header"
#pragma once
#pragma clang diagnostic pop

#include "HycType.h"
#include "StdStringUtility.h"

#include <string>
#include <toml11\toml.hpp>

namespace hyc {
namespace text {

using TomlNode = toml::value;

inline bool loadTomlAndParse(TomlNode &OutRoot,
                             const std::string &Path,
                             std::string &OutErrorMessage) {
  bool Result = true;

  try {
    OutRoot = toml::parse(Path);
  } catch (const std::runtime_error &err) {
    OutErrorMessage = err.what();
    Result = false;
  } catch (const toml::syntax_error &err) {
    OutErrorMessage = err.what();
    Result = false;
  } catch (...) {
    OutErrorMessage = "parse failed with unhandled exception";
    Result = false;
  }

  return Result;
}

inline bool loadTomlAndParse(TomlNode &OutRoot,
                             cstring CPath,
                             std::string &OutErrorMessage) {
  bool Result = true;

  try {
    OutRoot = toml::parse(CPath);
  } catch (const std::runtime_error &err) {
    OutErrorMessage = err.what();
    Result = false;
  } catch (const toml::syntax_error &err) {
    OutErrorMessage = err.what();
    Result = false;
  } catch (...) {
    OutErrorMessage = "parse failed with unhandled exception";
    Result = false;
  }

  return Result;
}

inline bool getNextTomlNode(const TomlNode &From,
                            const std::string &To,
                            TomlNode &OutNode) {
  bool Exist = From.contains(To);
  if (!Exist) {
    return false;
  }

  OutNode = toml::find(From, To);
  return true;
}

inline bool
getNextTomlNode(const TomlNode &From, cstring To, TomlNode &OutNode) {
  bool Exist = From.contains(To);
  if (!Exist) {
    return false;
  }

  OutNode = toml::find(From, To);
  return true;
}

inline bool
getTomlNode(const TomlNode &From, const std::string &To, TomlNode &OutNode) {
  std::vector<std::string> Paths = {};
  hyc::string::split(To, '.', Paths);

  TomlNode Now = From;
  TomlNode Next = {};

  for (const auto &Path : Paths) {
    if (!getNextTomlNode(Now, Path, Next)) {
      return false;
    } else {
      Now = Next;
    }
  }

  OutNode = Next;
  return true;
}

inline bool getTomlNode(const TomlNode &From, cstring To, TomlNode &OutNode) {
  std::vector<std::string> Paths = {};
  hyc::string::split(To, '.', Paths);

  TomlNode Now = From;
  TomlNode Next = {};

  for (const auto &Path : Paths) {
    if (!getNextTomlNode(Now, Path, Next)) {
      return false;
    } else {
      Now = Next;
    }
  }

  OutNode = Next;
  return true;
}

template <typename T>
T getAs(const TomlNode &Node) {
  return toml::get<T>(Node);
}

} // namespace text
} // namespace hyc
