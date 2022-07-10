#include "TcpLoggerConnection.h"

#include "PrintLog.h"

#include <FormatUtility.h>
#include <TomlUtility.h>

namespace tcp = hyc::tcp;
namespace str = hyc::str;
namespace tom = hyc::text;

std::list<TcpLoggerConnection::LOG> TcpLoggerConnection::LogInfoList = {};

TcpLoggerConnection::TcpLoggerConnection()
    : TcpConnectionInterface(), LoggerEnabledFlag(false), SocketPtr({}),
      ConnectAddress({}), LoggerProcessInfo({}) {}

TcpLoggerConnection::~TcpLoggerConnection() {}

void TcpLoggerConnection::insertNewLogMessage(int LogLevel,
                                              const std::string &LogMessage) {
  LogInfoList.push_back({LogLevel, LogMessage});
}

bool TcpLoggerConnection::createConnection() {
  if (__argc > 1) {
    for (int I = 1; I < __argc; I++) {
      if (std::string(__argv[I]) == "-L") {
        LoggerEnabledFlag = true;
      }
    }
  }

  if (!LoggerEnabledFlag) {
    return true;
  }

  tom::TomlNode LevelConfigRoot = {};
  std::string Message = "";
  if (!tom::loadTomlAndParse(LevelConfigRoot,
                             ".\\Assets\\Configs\\hyc-logger-config.toml",
                             Message)) {
    assert(false && "failed to parse logging level config file");
    return false;
  }

  switch (tom::getAs<int>(LevelConfigRoot["logger"]["output-level"])) {
  case 0:
    Message = "04_HycLogger.exe -l debug";
    break;
  case 1:
    Message = "04_HycLogger.exe -l message";
    break;
  case 2:
    Message = "04_HycLogger.exe -l warning";
    break;
  case 3:
    Message = "04_HycLogger.exe -l error";
    break;
  default:
    break;
  }

  STARTUPINFO SI = {};
  ZeroMemory(&SI, sizeof(SI));
  ZeroMemory(&LoggerProcessInfo, sizeof(LoggerProcessInfo));
  SI.cb = sizeof(SI);

  char *CmdLine = const_cast<char *>(Message.data());
  BOOL ProcessCreateFlag = CreateProcessA(NULL, CmdLine, NULL, NULL, FALSE, 0,
                                          NULL, NULL, &SI, &LoggerProcessInfo);

  if (!ProcessCreateFlag) {
    auto Error = GetLastError();
    assert(ProcessCreateFlag && "fail to create HycLogger");
    (void)Error;
    return false;
  }

  SocketPtr = tcp::TcpSocket::create(tcp::ADDRFAM::INET);
  ConnectAddress = tcp::createIPv4FromString("127.0.0.1:32580");
  int ConnectError = SocketPtr.get()->connect(*ConnectAddress);
  bool Result = ConnectError ? false : true;

  return Result;
}

void TcpLoggerConnection::terminateConnection() {
  if (LoggerEnabledFlag) {
    SocketPtr->sendAs<int>(-1);
    SocketPtr->sendAs<std::string>("/STOP_LOGGER");
    WaitForSingleObject(LoggerProcessInfo.hProcess, INFINITE);
    CloseHandle(LoggerProcessInfo.hProcess);
    CloseHandle(LoggerProcessInfo.hThread);
  }
}

void TcpLoggerConnection::executeConnection() {
  if (LoggerEnabledFlag) {
    for (const auto &LogMessage : LogInfoList) {
      SocketPtr->sendAs<int>(LogMessage.LogLevel);
      SocketPtr->sendAs<std::string>(LogMessage.LogMessage);
    }
    LogInfoList.clear();
  }
}
