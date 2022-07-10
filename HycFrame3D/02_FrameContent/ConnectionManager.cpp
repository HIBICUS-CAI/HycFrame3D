#include "ConnectionManager.h"

#include "TcpLoggerConnection.h"

#include <FormatUtility.h>

bool ConnectionManager::initConnections() {
  bool Result = hyc::tcp::sockSysStartUp();
  if (!Result) {
    return false;
  }

  TcpConnectionsMap.insert({TCP_TYPE::LOGGER, new TcpLoggerConnection()});

  for (auto &Connection : TcpConnectionsMap) {
    if (!Connection.second->createConnection()) {
      Result = false;
      break;
    }
  }

  if (!Result) {
    for (auto &Connection : TcpConnectionsMap) {
      delete Connection.second;
    }
    TcpConnectionsMap.clear();
  }

  return Result;
}

void ConnectionManager::executeConnections() {
  for (auto &Connection : TcpConnectionsMap) {
    Connection.second->executeConnection();
  }
}

void ConnectionManager::terminateConnections() {
  for (auto &Connection : TcpConnectionsMap) {
    Connection.second->terminateConnection();
    delete Connection.second;
  }
  TcpConnectionsMap.clear();
  hyc::tcp::sockSysTerminate();
}
