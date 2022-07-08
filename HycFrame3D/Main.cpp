#include "RootSystem.h"
#include <TcpUtility.h>

int WinMain(_In_ HINSTANCE Instance,
            _In_opt_ HINSTANCE PrevInstance,
            _In_ LPSTR CmdLine,
            _In_ int CmdShow) {
  RootSystem Root;

  if (Root.startUp(Instance, CmdShow)) {
    Root.runGameLoop();
  }

  Root.cleanAndStop();

  tcp::sockSysStartUp();
  auto Sock = tcp::TcpSocket::create(tcp::ADDRFAM::INET);
  auto Addr = tcp::createIPv4FromString("127.0.0.1:32580");
  Sock->tcpConnect(*Addr);

  return 0;
}
