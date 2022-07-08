#include <TcpUtility.h>

#include <cstdio>

int main() {
  SetConsoleTitleA("HycFrame3D Logger");

  HANDLE ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_FONT_INFOEX FontInfo = {};
  ZeroMemory(&FontInfo, sizeof(FontInfo));
  FontInfo.cbSize = sizeof(FontInfo);
  FontInfo.dwFontSize.X = 0;
  FontInfo.dwFontSize.Y = 24;
  FontInfo.FontWeight = 400;
  FontInfo.FontFamily = TMPF_TRUETYPE;
  std::wcscpy(FontInfo.FaceName, L"Ubuntu");
  SetConsoleOutputCP(437);
  if (!SetCurrentConsoleFontEx(ConsoleHandle, FALSE, &FontInfo)) {
    std::printf("Failed to change the font: %d\n",
                static_cast<int>(GetLastError()));
  }

  hyc::tcp::sockSysStartUp();

  {
    auto SocketPtr = hyc::tcp::TcpSocket::create(hyc::tcp::ADDRFAM::INET);
    auto ListenAddressPtr = hyc::tcp::createIPv4FromString("127.0.0.1:32580");
    SocketPtr->bind(*ListenAddressPtr);
    SocketPtr->listen();
    hyc::tcp::SocketAddress Addr = {};
    auto AcceptedSocketPtr = SocketPtr->accept(Addr);

    while (true) {
      std::string RecvStr = "";
      AcceptedSocketPtr->receiveAs<std::string>(RecvStr);
      if (RecvStr == "/STOP_LOGGER") {
        break;
      } else {
        std::printf("hello world and %s\n", RecvStr.c_str());
      }
    }
  }

  hyc::tcp::sockSysTerminate();

  return 0;
}
