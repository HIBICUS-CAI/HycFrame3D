#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-once-outside-header"
#endif // __clang__
#pragma once
#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#include <cassert>
#include <codecvt>
#include <cstdio>
#include <locale>
#include <memory>
#include <string>
#include <vector>

#define TCP_RECV(_SOCKET_PTR, _START_POS, _HAS_READ, _NEED_SIZE)               \
  while (_HAS_READ < _NEED_SIZE) {                                             \
    _HAS_READ += _SOCKET_PTR->tcpReceive(                                      \
        _START_POS + _HAS_READ, _NEED_SIZE - static_cast<size_t>(_HAS_READ));  \
  }

namespace tcp {

enum class ADDRFAM { INET = AF_INET, INET6 = AF_INET6 };

int getWsaError() { return WSAGetLastError(); }

void generateErrorInfo(::std::string &InOutBaseErrorMessage) {
  void *Message = nullptr;
  DWORD ErrorNum = getWsaError();
  std::string OriginCallStack = InOutBaseErrorMessage;

  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, ErrorNum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 reinterpret_cast<char *>(&Message), 0, NULL);

  InOutBaseErrorMessage = std::to_string(ErrorNum) + " : ";
  InOutBaseErrorMessage += std::string(static_cast<char *>(Message)) + " - ";
  InOutBaseErrorMessage += OriginCallStack;
}

bool sockSysStartUp(std::string *OutError = nullptr) {
  WSADATA WsaData = {};
  int Res = WSAStartup(MAKEWORD(2, 2), &WsaData);
  if (Res != NO_ERROR) {
    if (OutError) {
      *OutError = "Socket Starting Up";
      generateErrorInfo(*OutError);
    }
    return false;
  } else {
    return true;
  }
}

void sockSysTerminate() { WSACleanup(); }

class SocketAddress {
private:
  sockaddr SocketAddressData;

public:
  SocketAddress(uint32_t Address, uint16_t Port);

  SocketAddress(const sockaddr &Source);

  SocketAddress();

  bool operator==(const SocketAddress &Another) const;

  size_t getHash() const;

  uint32_t getSize() const;

  ::std::string toString() const;

private:
  friend class TcpSocket;
  friend class UdpSocket;

  uint32_t &getIP4Ref();

  const uint32_t &getIP4Ref() const;

  sockaddr_in *getAsSockAddrIn();

  const sockaddr_in *getAsSockAddrIn() const;
};

SocketAddress::SocketAddress(uint32_t Address, uint16_t Port) {
  getAsSockAddrIn()->sin_family = AF_INET;
  getIP4Ref() = htonl(Address);
  getAsSockAddrIn()->sin_port = htons(Port);
}

SocketAddress::SocketAddress(const sockaddr &Source) {
  memcpy(&SocketAddressData, &Source, sizeof(sockaddr));
}

SocketAddress::SocketAddress() {
  getAsSockAddrIn()->sin_family = AF_INET;
  getIP4Ref() = INADDR_ANY;
  getAsSockAddrIn()->sin_port = 0;
}

bool SocketAddress::operator==(const SocketAddress &Another) const {
  return (SocketAddressData.sa_family == Another.SocketAddressData.sa_family &&
          getAsSockAddrIn()->sin_port == Another.getAsSockAddrIn()->sin_port) &&
         (getIP4Ref() == Another.getIP4Ref());
}

size_t SocketAddress::getHash() const {
  return static_cast<size_t>(
      (getIP4Ref()) |
      ((static_cast<uint32_t>(getAsSockAddrIn()->sin_port)) << 13) |
      SocketAddressData.sa_family);
}

uint32_t SocketAddress::getSize() const { return sizeof(sockaddr); }

::std::string SocketAddress::toString() const {
  const sockaddr_in *AddrPtr = getAsSockAddrIn();
  char Temp[128] = "", Result[128] = "";
  InetNtopA(AddrPtr->sin_family, const_cast<in_addr *>(&AddrPtr->sin_addr),
            Temp, sizeof(Temp));

  ::std::sprintf(Result, "%s:%d", Temp, ntohs(AddrPtr->sin_port));
  return ::std::string(Result);
}

uint32_t &SocketAddress::getIP4Ref() {
  return *reinterpret_cast<uint32_t *>(
      &(getAsSockAddrIn()->sin_addr.S_un.S_addr));
}

const uint32_t &SocketAddress::getIP4Ref() const {
  return *reinterpret_cast<const uint32_t *>(
      &(getAsSockAddrIn()->sin_addr.S_un.S_addr));
}

sockaddr_in *SocketAddress::getAsSockAddrIn() {
  return reinterpret_cast<sockaddr_in *>(&SocketAddressData);
}

const sockaddr_in *SocketAddress::getAsSockAddrIn() const {
  return reinterpret_cast<const sockaddr_in *>(&SocketAddressData);
}

using SocketAddressPtr = std::shared_ptr<class SocketAddress>;

SocketAddressPtr createIPv4FromString(const std::string AddrInfo,
                                      std::string &ErrorMessage) {
  auto Pos = AddrInfo.find_last_of(':');
  std::string Host, Service;
  if (Pos != std::string::npos) {
    Host = AddrInfo.substr(0, Pos);
    Service = AddrInfo.substr(Pos + 1);
  } else {
    Host = AddrInfo;
    Service = "0";
  }
  addrinfo Hint;
  memset(&Hint, 0, sizeof(Hint));
  Hint.ai_family = AF_INET;

  addrinfo *Result;
  int Error = getaddrinfo(Host.c_str(), Service.c_str(), &Hint, &Result);
  if (Error != 0) {
    ErrorMessage = "createIPv4FromString";
    generateErrorInfo(ErrorMessage);
    if (Result) {
      freeaddrinfo(Result);
    }
    return nullptr;
  }

  while (!Result->ai_addr && Result->ai_next) {
    Result = Result->ai_next;
  }

  if (!Result->ai_addr) {
    return nullptr;
  }

  auto ToRet = std::make_shared<SocketAddress>(*Result->ai_addr);

  freeaddrinfo(Result);

  return ToRet;
}

using TcpSocketPtr = std::shared_ptr<class TcpSocket>;

class TcpSocket {
private:
  SOCKET RawSocket;

private:
  TcpSocket(SOCKET Socket) : RawSocket(Socket) {}

public:
  ~TcpSocket() { closesocket(RawSocket); }

  static TcpSocketPtr create(ADDRFAM Family) {
    SOCKET S = socket(static_cast<int>(Family), SOCK_STREAM, IPPROTO_TCP);

    if (S != INVALID_SOCKET) {
      return TcpSocketPtr(new TcpSocket(S));
    } else {
      std::string Error = "createTcpSocket";
      generateErrorInfo(Error);
#ifdef _DEBUG
      assert(false);
#endif // _DEBUG
      (void)Error;
      return nullptr;
    }
  }

  int tcpConnect(const SocketAddress &Addr) {
    int Err = connect(RawSocket, &Addr.SocketAddressData, Addr.getSize());

    if (Err < 0) {
      std::string Error = "tcpConnect";
      generateErrorInfo(Error);
#ifdef _DEBUG
      assert(false);
#endif // _DEBUG
      (void)Error;
      return -getWsaError();
    }

    return NO_ERROR;
  }

  int tcpBind(const SocketAddress &Addr) {
    int Err = bind(RawSocket, &Addr.SocketAddressData, Addr.getSize());

    if (Err != 0) {
      std::string Error = "tcpBind";
      generateErrorInfo(Error);
#ifdef _DEBUG
      assert(false);
#endif // _DEBUG
      (void)Error;
      return getWsaError();
    }

    return NO_ERROR;
  }

  int tcpListen(int BackLog = 32) {
    int err = listen(RawSocket, BackLog);

    if (err < 0) {
      std::string Error = "tcpListen";
      generateErrorInfo(Error);
#ifdef _DEBUG
      assert(false);
#endif // _DEBUG
      (void)Error;
      return getWsaError();
    }

    return NO_ERROR;
  }

  TcpSocketPtr tcpAccept(SocketAddress &From) {
    socklen_t Length = From.getSize();
    SOCKET NewSocket = accept(RawSocket, &From.SocketAddressData, &Length);

    if (NewSocket != INVALID_SOCKET) {
      return TcpSocketPtr(new TcpSocket(NewSocket));
    } else {
      std::string Error = "tcpAccept";
      generateErrorInfo(Error);
#ifdef _DEBUG
      assert(false);
#endif // _DEBUG
      (void)Error;
      return nullptr;
    }
  }

  int32_t tcpSend(const void *Data, size_t Size) {
    int BytesSentCount = send(RawSocket, static_cast<const char *>(Data),
                              static_cast<int>(Size), 0);

    if (BytesSentCount < 0) {
      std::string Error = "tcpSend";
      generateErrorInfo(Error);
#ifdef _DEBUG
      assert(false);
#endif // _DEBUG
      (void)Error;
      return -getWsaError();
    }

    return BytesSentCount;
  }

  int32_t tcpReceive(void *Buffer, size_t Size) {
    int BytesReceivedCount =
        recv(RawSocket, static_cast<char *>(Buffer), static_cast<int>(Size), 0);

    if (BytesReceivedCount < 0) {
      std::string Error = "tcpReceive";
      generateErrorInfo(Error);
#ifdef _DEBUG
      assert(false);
#endif // _DEBUG
      (void)Error;
      return -getWsaError();
    }

    return BytesReceivedCount;
  }
};

} // namespace tcp

namespace std {
template <>
struct hash<tcp::SocketAddress> {
  size_t operator()(const tcp::SocketAddress &Address) const {
    return Address.getHash();
  }
};
} // namespace std
