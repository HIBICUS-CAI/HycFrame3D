#include "TcpLoggerConnection.h"

#include "PrintLog.h"

#include <FormatUtility.h>
#include <TomlUtility.h>

namespace tcp = hyc::tcp;
namespace str = hyc::str;
namespace tom = hyc::text;
namespace proc = hyc::proc;

std::list<TcpLoggerConnection::LOG> TcpLoggerConnection::LogInfoList = {};

TcpLoggerConnection::TcpLoggerConnection()
    : TcpConnectionInterface(), LoggerEnabledFlag(false), SocketPtr({}),
      ConnectAddress({}), LoggerProcessInfo{} {}

TcpLoggerConnection::~TcpLoggerConnection() {}

void TcpLoggerConnection::insertNewLogMessage(int LogLevel,
                                              const std::string &LogMessage) {
  LogInfoList.push_back({LogLevel, LogMessage});
}

bool TcpLoggerConnection::createConnection() {
  auto Args = proc::getCommandLineArgs();
  for (size_t I = 1; I < Args.size(); I++) {
    if (Args[I] == "-L") {
      LoggerEnabledFlag = true;
    }
  }

  if (!LoggerEnabledFlag) {
    return true;
  }

  tom::TomlNode LevelConfigRoot = {};
  std::string Message = "";
  if (!tom::loadTomlAndParse(LevelConfigRoot,
                             "./Assets/Configs/hyc-logger-config.toml",
                             Message)) {
    assert(false && "failed to parse logging level config file");
    return false;
  }

  switch (tom::getAs<int>(LevelConfigRoot["logger"]["output-level"])) {
  case 0:
    Message = "debug";
    break;
  case 1:
    Message = "message";
    break;
  case 2:
    Message = "warning";
    break;
  case 3:
    Message = "error";
    break;
  default:
    break;
  }

  std::string LoggerPath = proc::getExecutableDirectory() + "/04_HycLogger" +
                           proc::EXECUTABLE_SUFFIX;
  bool ProcessCreateFlag =
      proc::createProcess(LoggerPath, {"-l", Message}, LoggerProcessInfo);

  if (!ProcessCreateFlag) {
    assert(ProcessCreateFlag && "fail to create HycLogger");
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
    proc::waitAndCloseProcess(LoggerProcessInfo);
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
