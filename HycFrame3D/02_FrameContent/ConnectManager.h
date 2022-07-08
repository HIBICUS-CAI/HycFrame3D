#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-once-outside-header"
#endif // __clang__
#pragma once
#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

#include <MacroUtility.h>
#include <TcpUtility.h>

class ConnectManager {
  DEFINE_SINGLETON(ConnectManager)
private:
  hyc::tcp::TcpSocketPtr SocketPtr = {};
  hyc::tcp::SocketAddressPtr ConnectAddress = {};

public:
  bool init();
  void update();
  void cleanAndStop();
};
