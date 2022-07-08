#include "ConnectManager.h"

bool ConnectManager::init() {
  bool Result = hyc::tcp::sockSysStartUp();

  SocketPtr = hyc::tcp::TcpSocket::create(hyc::tcp::ADDRFAM::INET);
  ConnectAddress = hyc::tcp::createIPv4FromString("127.0.0.1:32580");
  SocketPtr.get()->tcpConnect(*ConnectAddress);

  return Result;
}

void ConnectManager::update() {}

void ConnectManager::cleanAndStop() { hyc::tcp::sockSysTerminate(); }
