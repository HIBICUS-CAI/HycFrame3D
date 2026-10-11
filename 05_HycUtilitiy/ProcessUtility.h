#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-once-outside-header"
#endif // __clang__
#pragma once
#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

#include "HycType.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN
#include <stdlib.h>
#include <windows.h>
#else
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <climits>
#include <fstream>
#include <iterator>
extern char **environ;
#endif // _WIN32

#include <string>
#include <vector>

namespace hyc {
namespace proc {

#ifdef _WIN32
constexpr cstring EXECUTABLE_SUFFIX = ".exe";
using ProcessHandle = PROCESS_INFORMATION;
#else
constexpr cstring EXECUTABLE_SUFFIX = "";
using ProcessHandle = pid_t;
#endif // _WIN32

inline std::vector<std::string> getCommandLineArgs() {
  std::vector<std::string> Args = {};
#ifdef _WIN32
  for (int I = 0; I < __argc; I++) {
    Args.emplace_back(__argv[I]);
  }
#else
  std::ifstream CmdLine("/proc/self/cmdline", std::ios::binary);
  std::string Raw((std::istreambuf_iterator<char>(CmdLine)),
                  std::istreambuf_iterator<char>());
  std::string::size_type Start = 0;
  while (Start < Raw.size()) {
    auto End = Raw.find('\0', Start);
    if (End == std::string::npos) {
      End = Raw.size();
    }
    Args.emplace_back(Raw.substr(Start, End - Start));
    Start = End + 1;
  }
#endif // _WIN32
  return Args;
}

inline std::string getExecutableDirectory() {
  std::string Path = "";
#ifdef _WIN32
  char Buffer[MAX_PATH] = "";
  auto Size = GetModuleFileNameA(nullptr, Buffer, MAX_PATH);
  Path.assign(Buffer, Size);
#else
  char Buffer[PATH_MAX] = "";
  auto Size = readlink("/proc/self/exe", Buffer, sizeof(Buffer) - 1);
  if (Size > 0) {
    Path.assign(Buffer, static_cast<size_t>(Size));
  }
#endif // _WIN32
  auto Pos = Path.find_last_of("/\\");
  if (Pos == std::string::npos) {
    return ".";
  }
  return Path.substr(0, Pos);
}

inline bool createProcess(const std::string &ExecutablePath,
                          const std::vector<std::string> &Args,
                          ProcessHandle &OutHandle) {
#ifdef _WIN32
  std::string CmdLine = "\"" + ExecutablePath + "\"";
  for (const auto &Arg : Args) {
    CmdLine += " " + Arg;
  }
  STARTUPINFOA SI = {};
  SI.cb = sizeof(SI);
  OutHandle = {};
  return CreateProcessA(nullptr, CmdLine.data(), nullptr, nullptr, FALSE, 0,
                        nullptr, nullptr, &SI, &OutHandle) != FALSE;
#else
  std::vector<char *> Argv = {};
  Argv.push_back(const_cast<char *>(ExecutablePath.c_str()));
  for (const auto &Arg : Args) {
    Argv.push_back(const_cast<char *>(Arg.c_str()));
  }
  Argv.push_back(nullptr);
  return posix_spawn(&OutHandle, ExecutablePath.c_str(), nullptr, nullptr,
                     Argv.data(), environ) == 0;
#endif // _WIN32
}

inline void waitAndCloseProcess(ProcessHandle &Handle) {
#ifdef _WIN32
  WaitForSingleObject(Handle.hProcess, INFINITE);
  CloseHandle(Handle.hProcess);
  CloseHandle(Handle.hThread);
#else
  int Status = 0;
  waitpid(Handle, &Status, 0);
#endif // _WIN32
}

} // namespace proc
} // namespace hyc
