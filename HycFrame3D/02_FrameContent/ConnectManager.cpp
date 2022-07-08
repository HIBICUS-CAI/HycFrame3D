#include "ConnectManager.h"

bool ConnectManager::init() {
  bool Result = hyc::tcp::sockSysStartUp();
  if (!Result) {
    return false;
  }

  STARTUPINFO SI = {};
  ZeroMemory(&SI, sizeof(SI));
  ZeroMemory(&LoggerProcessInfo, sizeof(LoggerProcessInfo));
  SI.cb = sizeof(SI);

  char CmdLine[] = {"04_HycLogger.exe"};
  BOOL ProcessCreateFlag = CreateProcessA(NULL, CmdLine, NULL, NULL, FALSE, 0,
                                          NULL, NULL, &SI, &LoggerProcessInfo);

  if (!ProcessCreateFlag) {
    auto Error = GetLastError();
    (void)Error;
    return false;
  }

  SocketPtr = hyc::tcp::TcpSocket::create(hyc::tcp::ADDRFAM::INET);
  ConnectAddress = hyc::tcp::createIPv4FromString("127.0.0.1:32580");
  int ConnectError = SocketPtr.get()->connect(*ConnectAddress);
  Result = ConnectError ? false : true;

  return Result;
}

void ConnectManager::update() {
  static int FrameCount = 0;
  SocketPtr->sendAs<std::string>("game frame : " +
                                 std::to_string(FrameCount++));
}

void ConnectManager::cleanAndStop() {
  SocketPtr->sendAs<std::string>("/STOP_LOGGER");
  WaitForSingleObject(LoggerProcessInfo.hProcess, INFINITE);
  CloseHandle(LoggerProcessInfo.hProcess);
  CloseHandle(LoggerProcessInfo.hThread);
  hyc::tcp::sockSysTerminate();
}
