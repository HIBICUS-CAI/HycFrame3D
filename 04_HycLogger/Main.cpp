#include <TcpUtility.h>

#include <cstdio>

int main() {
  hyc::tcp::sockSysStartUp();

  {
    auto SocketPtr = hyc::tcp::TcpSocket::create(hyc::tcp::ADDRFAM::INET);
    auto ListenAddressPtr = hyc::tcp::createIPv4FromString("127.0.0.1:32580");
    SocketPtr->tcpBind(*ListenAddressPtr);
    SocketPtr->tcpListen();
    hyc::tcp::SocketAddress Addr = {};
    auto AcceptedSocketPtr = SocketPtr->tcpAccept(Addr);

    while (true) {
      char Buffer[512] = "\0";
      ZeroMemory(&Buffer, sizeof(Buffer));
      int HasRead = AcceptedSocketPtr->tcpReceive(Buffer, 4);
      TCP_RECV(AcceptedSocketPtr, Buffer, HasRead, 4);
      int StrLen = *(reinterpret_cast<int *>(Buffer));
      ZeroMemory(&Buffer, sizeof(Buffer));
      HasRead = AcceptedSocketPtr->tcpReceive(Buffer, StrLen);
      TCP_RECV(AcceptedSocketPtr, Buffer, HasRead, StrLen);
      if (!std::strcmp(Buffer, "Stop Logger")) {
        break;
      } else {
        std::printf("hello world and %s\n", Buffer);
      }
    }
  }

  hyc::tcp::sockSysTerminate();
}
