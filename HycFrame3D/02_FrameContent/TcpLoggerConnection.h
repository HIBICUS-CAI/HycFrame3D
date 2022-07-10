#pragma once

#include <TcpConnectionInterface.h>

#include <TcpUtility.h>

#include <list>
#include <string>

class TcpLoggerConnection : public TcpConnectionInterface {
private:
  hyc::tcp::TcpSocketPtr SocketPtr;
  hyc::tcp::SocketAddressPtr ConnectAddress;
  PROCESS_INFORMATION LoggerProcessInfo;

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
