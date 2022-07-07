#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-once-outside-header"
#endif // __clang__
#pragma once
#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

#include "HycType.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#endif // __clang__
#include <rapidjson\document.h>
#include <rapidjson\filereadstream.h>
#include <rapidjson\istreamwrapper.h>
#include <rapidjson\pointer.h>
#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

#include <fstream>
#include <string>

namespace hyc {
namespace text {

using JsonFile = rapidjson::Document;
using JsonNode = rapidjson::Value *;

inline bool loadJsonAndParse(JsonFile &OutFile, const std::string &Path) {
  std::ifstream IfS(Path);
  rapidjson::IStreamWrapper ISW(IfS);
  OutFile.ParseStream(ISW);

  return !OutFile.HasParseError();
}

inline bool loadJsonAndParse(JsonFile &OutFile, cstring Path) {
  std::ifstream IfS(Path);
  rapidjson::IStreamWrapper ISW(IfS);
  OutFile.ParseStream(ISW);

  return !OutFile.HasParseError();
}

inline uint getJsonParseError(const JsonFile &File) {
  return static_cast<uint>(File.GetParseError());
}

inline JsonNode getJsonNode(JsonFile &File, const std::string &Path) {
  rapidjson::Pointer Ptr(Path.c_str());
  return rapidjson::GetValueByPointer(File, Ptr);
}

inline JsonNode getJsonNode(JsonFile &File, cstring Path) {
  rapidjson::Pointer Ptr(Path);
  return rapidjson::GetValueByPointer(File, Ptr);
}

template <typename T>
T GetAs(const JsonNode &Node) {
  return Node->Get<T>();
}

} // namespace text
} // namespace hyc
