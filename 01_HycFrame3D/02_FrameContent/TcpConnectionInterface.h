#pragma once

class TcpConnectionInterface {
public:
  TcpConnectionInterface() {}
  virtual ~TcpConnectionInterface() {}

  virtual bool createConnection() = 0;
  virtual void terminateConnection() = 0;
  virtual void executeConnection() = 0;
};
