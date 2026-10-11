#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-once-outside-header"
#endif // __clang__
#pragma once
#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

#include "FormatUtility.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#endif // _WIN32

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#ifndef _WIN32
using SOCKET = int;
constexpr SOCKET INVALID_SOCKET = -1;
constexpr int NO_ERROR = 0;
inline int closesocket(SOCKET Socket) { return close(Socket); }
#endif // !_WIN32

#ifdef MSG_NOSIGNAL
// do not raise SIGPIPE when the peer has been closed
constexpr int SEND_FLAGS = MSG_NOSIGNAL;
#else
constexpr int SEND_FLAGS = 0;
#endif // MSG_NOSIGNAL

#define TCP_RECV(_SOCKET_PTR, _START_POS, _HAS_READ, _NEED_SIZE)               \
  while (_HAS_READ < _NEED_SIZE) {                                             \
    _HAS_READ += _SOCKET_PTR->receive(                                         \
        _START_POS + _HAS_READ, _NEED_SIZE - static_cast<size_t>(_HAS_READ));  \
  }

namespace hyc {
namespace tcp {

enum class ADDRFAM { INET = AF_INET, INET6 = AF_INET6 };

#ifdef _WIN32
inline int getWsaError() { return WSAGetLastError(); }

inline void generateErrorInfo(const std::string &CallStack) {
  void *Message = nullptr;
  DWORD ErrorNum = getWsaError();

  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, ErrorNum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 reinterpret_cast<char *>(&Message), 0, NULL);

  std::string ErrorMessage =
      str::sFormat("{} : {} - {}", ErrorNum,
                   std::string(static_cast<char *>(Message)), CallStack);
  MessageBoxA(NULL, ErrorMessage.c_str(), "Tcp Error", MB_ABORTRETRYIGNORE);
  exit(-1);
}
#else
inline int getWsaError() { return errno; }

inline void generateErrorInfo(const std::string &CallStack) {
  int ErrorNum = getWsaError();
  std::string ErrorMessage = str::sFormat(
      "{} : {} - {}", ErrorNum, std::string(std::strerror(ErrorNum)), CallStack);
  std::fprintf(stderr, "[Tcp Error] %s\n", ErrorMessage.c_str());
  exit(-1);
}
#endif // _WIN32

#ifdef _WIN32
inline bool sockSysStartUp() {
  WSADATA WsaData = {};
  int Res = WSAStartup(MAKEWORD(2, 2), &WsaData);
  if (Res != NO_ERROR) {
    generateErrorInfo("sockSysStartUp::WSAStartUp");
    return false;
  } else {
    return true;
  }
}

inline void sockSysTerminate() { WSACleanup(); }
#else
inline bool sockSysStartUp() { return true; }

inline void sockSysTerminate() {}
#endif // _WIN32

class SocketAddress {
private:
  sockaddr SocketAddressData;

public:
  inline SocketAddress(uint32_t Address, uint16_t Port);

  inline SocketAddress(const sockaddr &Source);

  inline SocketAddress();

  inline bool operator==(const SocketAddress &Another) const;

  inline size_t getHash() const;

  inline uint32_t getSize() const;

  inline std::string toString() const;

private:
  friend class TcpSocket;
  friend class UdpSocket;

  inline uint32_t &getIP4Ref();

  inline const uint32_t &getIP4Ref() const;

  inline sockaddr_in *getAsSockAddrIn();

  inline const sockaddr_in *getAsSockAddrIn() const;
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

std::string SocketAddress::toString() const {
  const sockaddr_in *AddrPtr = getAsSockAddrIn();
  char Temp[128] = "", Result[128] = "";
  inet_ntop(AddrPtr->sin_family, &AddrPtr->sin_addr, Temp, sizeof(Temp));

  std::sprintf(Result, "%s:%d", Temp, ntohs(AddrPtr->sin_port));
  return std::string(Result);
}

uint32_t &SocketAddress::getIP4Ref() {
  return *reinterpret_cast<uint32_t *>(
      &(getAsSockAddrIn()->sin_addr.s_addr));
}

const uint32_t &SocketAddress::getIP4Ref() const {
  return *reinterpret_cast<const uint32_t *>(
      &(getAsSockAddrIn()->sin_addr.s_addr));
}

sockaddr_in *SocketAddress::getAsSockAddrIn() {
  return reinterpret_cast<sockaddr_in *>(&SocketAddressData);
}

const sockaddr_in *SocketAddress::getAsSockAddrIn() const {
  return reinterpret_cast<const sockaddr_in *>(&SocketAddressData);
}

using SocketAddressPtr = std::shared_ptr<class SocketAddress>;

inline SocketAddressPtr createIPv4FromString(const std::string AddrInfo) {
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
    generateErrorInfo("createIPv4FromString");
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
      generateErrorInfo("createTcpSocket");
      return nullptr;
    }
  }

  int connect(const SocketAddress &Addr) {
    int Err = ::connect(RawSocket, &Addr.SocketAddressData, Addr.getSize());

    if (Err < 0) {
      generateErrorInfo("tcpConnect");
      return -getWsaError();
    }

    return NO_ERROR;
  }

  int bind(const SocketAddress &Addr) {
    int Err = ::bind(RawSocket, &Addr.SocketAddressData, Addr.getSize());

    if (Err != 0) {
      generateErrorInfo("tcpBind");
      return getWsaError();
    }

    return NO_ERROR;
  }

  int listen(int BackLog = 32) {
    int err = ::listen(RawSocket, BackLog);

    if (err < 0) {
      generateErrorInfo("tcpListen");
      return getWsaError();
    }

    return NO_ERROR;
  }

  TcpSocketPtr accept(SocketAddress &From) {
    socklen_t Length = From.getSize();
    SOCKET NewSocket = ::accept(RawSocket, &From.SocketAddressData, &Length);

    if (NewSocket != INVALID_SOCKET) {
      return TcpSocketPtr(new TcpSocket(NewSocket));
    } else {
      generateErrorInfo("tcpAccept");
      return nullptr;
    }
  }

  int32_t send(const void *Data, size_t Size) {
    int BytesSentCount = ::send(RawSocket, static_cast<const char *>(Data),
                                static_cast<int>(Size), SEND_FLAGS);

    if (BytesSentCount < 0) {
      generateErrorInfo("tcpSend");
      return -getWsaError();
    }

    return BytesSentCount;
  }

  template <typename T>
  int32_t sendAs(const T &Data) {
    if constexpr (std::is_same_v<T, std::string>) {
      auto StrSize = Data.size();
      auto SentSize = sendAs<int>(static_cast<int>(StrSize));
      const void *DataPtr = Data.c_str();
      SentSize += send(DataPtr, StrSize);

      return SentSize;
    } else {
      const void *DataPtr = &Data;
      int DataSize = sizeof(T);

      return send(DataPtr, DataSize);
    }
  }

  int32_t receive(void *Buffer, size_t Size) {
    int BytesReceivedCount =
        recv(RawSocket, static_cast<char *>(Buffer), static_cast<int>(Size), 0);

    if (BytesReceivedCount < 0) {
      generateErrorInfo("tcpReceive");
      return -getWsaError();
    }

    return BytesReceivedCount;
  }

  template <typename T>
  int32_t receiveAs(T &OutData) {
    if constexpr (std::is_same_v<T, std::string>) {
      int StrSize = 0;
      auto LenReceived = receiveAs<int>(StrSize);
      TCP_RECV(this, &StrSize, LenReceived, sizeof(int));
      auto DataSize = static_cast<size_t>(StrSize);
      OutData.resize(DataSize);
      char *DataBuffer = const_cast<char *>(OutData.data());
      auto StrReceived = receive(DataBuffer, DataSize);
      TCP_RECV(this, DataBuffer, StrReceived, DataSize);
      return LenReceived + StrReceived;
    } else {
      auto DataSize = sizeof(T);
      auto Received = receive(&OutData, DataSize);
      TCP_RECV(this, &OutData, Received, DataSize);
      return Received;
    }
  }
};

} // namespace tcp
} // namespace hyc

namespace std {
template <>
struct hash<hyc::tcp::SocketAddress> {
  size_t operator()(const hyc::tcp::SocketAddress &Address) const {
    return Address.getHash();
  }
};
} // namespace std
