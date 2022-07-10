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

  static std::list<std::string> LogInfoList;

public:
  TcpLoggerConnection();
  virtual ~TcpLoggerConnection();

  static inline void insertNewLogMessage(const std::string &LogMessage);

public:
  virtual bool createConnection();
  virtual void terminateConnection();
  virtual void executeConnection();
};
