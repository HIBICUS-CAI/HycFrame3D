#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-once-outside-header"
#endif // __clang__
#pragma once
#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

#include "HycType.h"

#include <string>
#include <vector>

namespace hyc {
namespace string {

inline void split(const std::string &Source,
                  char Symbol,
                  std::vector<std::string> &OutResult) {
  OutResult.clear();
  std::string::size_type Pos1 = 0;
  std::string::size_type Pos2 = Source.find(Symbol);

  while (std::string::npos != Pos2) {
    OutResult.push_back(Source.substr(Pos1, Pos2 - Pos1));
    Pos1 = Pos2 + 1;
    Pos2 = Source.find(Symbol, Pos1);
  }

  if (Pos1 != Source.length()) {
    OutResult.push_back(Source.substr(Pos1));
  }
}

inline void
split(cstring Source, char Symbol, std::vector<std::string> &OutResult) {
  OutResult.clear();
  std::string SourceStr(Source);
  std::string::size_type Pos1 = 0;
  std::string::size_type Pos2 = SourceStr.find(Symbol);

  while (std::string::npos != Pos2) {
    OutResult.push_back(SourceStr.substr(Pos1, Pos2 - Pos1));
    Pos1 = Pos2 + 1;
    Pos2 = SourceStr.find(Symbol, Pos1);
  }

  if (Pos1 != SourceStr.length()) {
    OutResult.push_back(SourceStr.substr(Pos1));
  }
}

inline void split(const std::string &Source,
                  const std::string &Symbol,
                  std::vector<std::string> &OutResult) {
  OutResult.clear();
  std::string::size_type Pos1 = 0;
  std::string::size_type Pos2 = Source.find(Symbol);
  auto SymbolSize = Symbol.size();

  while (std::string::npos != Pos2) {
    OutResult.push_back(Source.substr(Pos1, Pos2 - Pos1));
    Pos1 = Pos2 + SymbolSize;
    Pos2 = Source.find(Symbol, Pos1);
  }

  if (Pos1 != Source.length()) {
    OutResult.push_back(Source.substr(Pos1));
  }
}

inline void
split(cstring Source, cstring Symbol, std::vector<std::string> &OutResult) {
  OutResult.clear();
  std::string SourceStr(Source);
  std::string SymbolStr(Symbol);
  auto SymbolSize = SymbolStr.size();
  std::string::size_type Pos1 = 0;
  std::string::size_type Pos2 = SourceStr.find(SymbolStr);

  while (std::string::npos != Pos2) {
    OutResult.push_back(SourceStr.substr(Pos1, Pos2 - Pos1));
    Pos1 = Pos2 + SymbolSize;
    Pos2 = SourceStr.find(SymbolStr, Pos1);
  }

  if (Pos1 != SourceStr.length()) {
    OutResult.push_back(SourceStr.substr(Pos1));
  }
}

} // namespace string
} // namespace hyc
