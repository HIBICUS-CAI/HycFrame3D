#include <TcpUtility.h>

#include <spdlog\sinks\basic_file_sink.h>
#include <spdlog\spdlog.h>

#include <chrono>
#include <ctime>
#include <cstring>
#include <unordered_map>

int main(int Argc, char **Args) {
  SetConsoleTitleA("HycFrame3D Logger");

  HANDLE ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_FONT_INFOEX FontInfo = {};
  ZeroMemory(&FontInfo, sizeof(FontInfo));
  FontInfo.cbSize = sizeof(FontInfo);
  FontInfo.dwFontSize.X = 0;
  FontInfo.dwFontSize.Y = 20;
  FontInfo.FontWeight = 400;
  FontInfo.FontFamily = TMPF_TRUETYPE;
  std::wcscpy(FontInfo.FaceName, L"Courier New");
  SetConsoleOutputCP(GetACP());
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

    std::string LoggingFileName = "";
    {
      std::time_t CTime = std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now());
      LoggingFileName = "Logs/" + std::string(std::ctime(&CTime)) + "txt";
      std::replace(LoggingFileName.begin(), LoggingFileName.end(), ' ', '_');
      std::replace(LoggingFileName.begin(), LoggingFileName.end(), ':', '_');
      std::replace(LoggingFileName.begin(), LoggingFileName.end(), '\n', '.');
    }
    auto Hyc3DLogger = spdlog::basic_logger_mt("Hyc3DLogger", LoggingFileName);
    spdlog::set_level(spdlog::level::trace);
    Hyc3DLogger->set_level(spdlog::level::trace);
    Hyc3DLogger->flush_on(spdlog::level::trace);

    if (Argc != 3) {
      assert(false && "invalid args");
      return -1;
    }

    spdlog::info("cmdline information : {} {} {}", Args[0], Args[1], Args[2]);
    Hyc3DLogger->info("cmdline information : {} {} {}", Args[0], Args[1],
                      Args[2]);

    spdlog::level::level_enum FinalLevel = spdlog::level::trace;
    if (!std::strcmp(Args[2], "debug")) {
      FinalLevel = spdlog::level::debug;
    } else if (!std::strcmp(Args[2], "message")) {
      FinalLevel = spdlog::level::info;
    } else if (!std::strcmp(Args[2], "warning")) {
      FinalLevel = spdlog::level::warn;
    } else if (!std::strcmp(Args[2], "error")) {
      FinalLevel = spdlog::level::err;
    }

    spdlog::set_level(FinalLevel);
    Hyc3DLogger->set_level(FinalLevel);
    Hyc3DLogger->flush_on(FinalLevel);

    const std::unordered_map<int, spdlog::level::level_enum> LoggingLevel = {
        {0, spdlog::level::debug},
        {1, spdlog::level::info},
        {2, spdlog::level::warn},
        {3, spdlog::level::err}};

    while (true) {
      int LogLevel = 0;
      std::string LogMessage = "";
      AcceptedSocketPtr->receiveAs<int>(LogLevel);
      AcceptedSocketPtr->receiveAs<std::string>(LogMessage);
      if (LogLevel == -1) {
        if (LogMessage == "/STOP_LOGGER") {
          break;
        }
      } else {
        spdlog::log(LoggingLevel.at(LogLevel), LogMessage);
        Hyc3DLogger->log(LoggingLevel.at(LogLevel), LogMessage);
      }
    }
  }

  hyc::tcp::sockSysTerminate();

  return 0;
}
