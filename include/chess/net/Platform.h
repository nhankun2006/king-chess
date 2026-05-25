#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdexcept>

/// Cross-platform socket abstraction.
/// Wraps the ~5 API differences between POSIX sockets and Winsock2
/// so that NetworkSession.cpp can use a single, platform-agnostic API.

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #ifdef _MSC_VER
    #pragma comment(lib, "ws2_32.lib")
  #endif

  using SocketHandle = SOCKET;
  constexpr SocketHandle kInvalidSocket = INVALID_SOCKET;

  inline int closeSocket(SocketHandle s) { return closesocket(s); }
  inline int getLastSocketError() { return WSAGetLastError(); }


  /// RAII guard — calls WSAStartup on construction, WSACleanup on destruction.
  /// Create one instance at application startup (e.g. in main() or LobbyScene).
  struct WinsockInit {
    WinsockInit() {
      WSADATA data;
      if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        throw std::runtime_error("WSAStartup failed");
      }
    }
    ~WinsockInit() { WSACleanup(); }

    WinsockInit(const WinsockInit &) = delete;
    WinsockInit &operator=(const WinsockInit &) = delete;
  };

#else // Linux / macOS
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <cerrno>

  using SocketHandle = int;
  constexpr SocketHandle kInvalidSocket = -1;

  inline int closeSocket(SocketHandle s) { return close(s); }
  inline int getLastSocketError() { return errno; }

  /// No-op on POSIX platforms — Winsock initialization is not needed.
  struct WinsockInit {};

#endif

#endif // PLATFORM_H
