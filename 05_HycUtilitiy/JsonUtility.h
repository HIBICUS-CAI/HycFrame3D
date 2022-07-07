#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-once-outside-header"
#pragma once
#pragma clang diagnostic pop

#include "HycType.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#include <rapidjson\document.h>
#include <rapidjson\filereadstream.h>
#include <rapidjson\istreamwrapper.h>
#include <rapidjson\pointer.h>
#pragma clang diagnostic pop

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
