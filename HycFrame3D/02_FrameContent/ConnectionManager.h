#pragma once

#include "TcpConnectionInterface.h"

#include <MacroUtility.h>
#include <TcpUtility.h>

#include <map>
#include <string>

class ConnectionManager {
  DEFINE_SINGLETON(ConnectionManager)
public:
  enum class TCP_TYPE {
    LOGGER,

    SIZE
  };

private:
  std::map<TCP_TYPE, TcpConnectionInterface *> TcpConnectionsMap = {};

public:
  bool initConnections();
  void executeConnections();
  void terminateConnections();
};
