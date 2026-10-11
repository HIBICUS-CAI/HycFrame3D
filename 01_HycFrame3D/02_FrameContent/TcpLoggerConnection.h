#pragma once

#include <TcpConnectionInterface.h>

#include <ProcessUtility.h>
#include <TcpUtility.h>

#include <list>
#include <string>

class TcpLoggerConnection : public TcpConnectionInterface {
private:
  bool LoggerEnabledFlag;
  hyc::tcp::TcpSocketPtr SocketPtr;
  hyc::tcp::SocketAddressPtr ConnectAddress;
  hyc::proc::ProcessHandle LoggerProcessInfo;

  struct LOG {
    int LogLevel;
    std::string LogMessage;
  };
  static std::list<LOG> LogInfoList;

public:
  TcpLoggerConnection();
  virtual ~TcpLoggerConnection();

  static void insertNewLogMessage(int LogLevel, const std::string &LogMessage);

public:
  virtual bool createConnection();
  virtual void terminateConnection();
  virtual void executeConnection();
};
