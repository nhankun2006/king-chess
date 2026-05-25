#include "chess/net/NetworkSession.h"

#include <cstring>
#include <iostream>

// Platform-specific includes for getLocalIPAddress()
#ifdef _WIN32
#include <iphlpapi.h>
#ifdef _MSC_VER
#pragma comment(lib, "iphlpapi.lib")
#endif
#else
#include <ifaddrs.h>
#include <net/if.h>
#endif

// ── Helper: set SO_REUSEADDR on a socket ────────────────────────────────────

static bool setReuseAddr(SocketHandle sock) {
  int opt = 1;
  return setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                    reinterpret_cast<const char *>(&opt),
                    static_cast<socklen_t>(sizeof(opt))) == 0;
}

// ── Constructor / Destructor ────────────────────────────────────────────────

NetworkSession::NetworkSession() = default;

NetworkSession::~NetworkSession() { disconnect(); }

// ── Connection lifecycle ────────────────────────────────────────────────────

bool NetworkSession::hostAndWaitForClient(int port) {
  // Create TCP socket
  serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket_ == kInvalidSocket) {
    std::cerr << "[NetworkSession] Failed to create server socket\n";
    return false;
  }

  setReuseAddr(serverSocket_);

  // Bind to all interfaces on the given port
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(static_cast<uint16_t>(port));

  if (bind(serverSocket_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) <
      0) {
    std::cerr << "[NetworkSession] bind() failed: " << getLastSocketError()
              << "\n";
    closeAndReset(serverSocket_);
    return false;
  }

  if (listen(serverSocket_, 1) < 0) {
    std::cerr << "[NetworkSession] listen() failed: " << getLastSocketError()
              << "\n";
    closeAndReset(serverSocket_);
    return false;
  }

  std::cout << "[NetworkSession] Listening on port " << port
            << ", waiting for client...\n";

  // Accept one client (blocking)
  sockaddr_in clientAddr{};
  socklen_t clientLen = sizeof(clientAddr);
  peerSocket_ = accept(serverSocket_,
                        reinterpret_cast<sockaddr *>(&clientAddr), &clientLen);

  if (peerSocket_ == kInvalidSocket) {
    std::cerr << "[NetworkSession] accept() failed: " << getLastSocketError()
              << "\n";
    closeAndReset(serverSocket_);
    return false;
  }

  // Server socket no longer needed — we have the peer connection
  closeAndReset(serverSocket_);

  connected_ = true;
  shouldStop_ = false;

  std::cout << "[NetworkSession] Client connected from "
            << inet_ntoa(clientAddr.sin_addr) << "\n";

  // Start background receive thread
  receiveThread_ = std::thread(&NetworkSession::receiveLoop, this);

  return true;
}

bool NetworkSession::connectToHost(const std::string &ip, int port) {
  peerSocket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (peerSocket_ == kInvalidSocket) {
    std::cerr << "[NetworkSession] Failed to create client socket\n";
    return false;
  }

  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(static_cast<uint16_t>(port));

  if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0) {
    std::cerr << "[NetworkSession] Invalid IP address: " << ip << "\n";
    closeAndReset(peerSocket_);
    return false;
  }

  std::cout << "[NetworkSession] Connecting to " << ip << ":" << port
            << "...\n";

  if (connect(peerSocket_, reinterpret_cast<sockaddr *>(&serverAddr),
              sizeof(serverAddr)) < 0) {
    std::cerr << "[NetworkSession] connect() failed: " << getLastSocketError()
              << "\n";
    closeAndReset(peerSocket_);
    return false;
  }

  connected_ = true;
  shouldStop_ = false;

  std::cout << "[NetworkSession] Connected to host at " << ip << "\n";

  // Start background receive thread
  receiveThread_ = std::thread(&NetworkSession::receiveLoop, this);

  return true;
}

// ── Game communication ──────────────────────────────────────────────────────

void NetworkSession::sendMove(const Move &move) {
  auto data = Protocol::encodeMoveMessage(move);
  if (!sendRaw(data)) {
    std::cerr << "[NetworkSession] Failed to send move\n";
  }
}

void NetworkSession::sendMessage(Protocol::MessageType type) {
  auto data = Protocol::encodeMessage(type);
  if (!sendRaw(data)) {
    std::cerr << "[NetworkSession] Failed to send message\n";
  }
}

std::optional<Protocol::Message> NetworkSession::tryReceive() {
  return incomingQueue_.tryPop();
}

bool NetworkSession::isConnected() const { return connected_; }

void NetworkSession::disconnect() {
  // Early exit if already disconnected — makes this safe to call
  // multiple times (e.g. explicit call + destructor).
  bool expected = true;
  if (!connected_.compare_exchange_strong(expected, false)) {
    return;
  }

  shouldStop_ = true;

  // Shutting down the socket unblocks any blocking recv() in the receive thread
  if (peerSocket_ != kInvalidSocket) {
#ifdef _WIN32
    shutdown(peerSocket_, SD_BOTH);
#else
    shutdown(peerSocket_, SHUT_RDWR);
#endif
    closeAndReset(peerSocket_);
  }

  closeAndReset(serverSocket_);

  if (receiveThread_.joinable()) {
    receiveThread_.join();
  }

  incomingQueue_.clear();
}

// ── Local IP address discovery ──────────────────────────────────────────────

std::string NetworkSession::getLocalIPAddress() {
#ifdef _WIN32
  // Windows: use GetAdaptersAddresses
  ULONG bufferSize = 15000;
  std::vector<uint8_t> buffer(bufferSize);
  auto *addresses =
      reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data());

  ULONG result = GetAdaptersAddresses(AF_INET, 0, nullptr, addresses,
                                      &bufferSize);
  if (result != NO_ERROR) return "127.0.0.1";

  for (auto *adapter = addresses; adapter; adapter = adapter->Next) {
    if (adapter->OperStatus != IfOperStatusUp) continue;
    if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK) continue;

    for (auto *unicast = adapter->FirstUnicastAddress; unicast;
         unicast = unicast->Next) {
      auto *sa =
          reinterpret_cast<sockaddr_in *>(unicast->Address.lpSockaddr);
      if (sa->sin_family == AF_INET) {
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sa->sin_addr, ipStr, sizeof(ipStr));
        return std::string(ipStr);
      }
    }
  }
  return "127.0.0.1";

#else
  // Linux / macOS: use getifaddrs
  struct ifaddrs *ifAddrList = nullptr;
  if (getifaddrs(&ifAddrList) == -1) return "127.0.0.1";

  std::string result = "127.0.0.1";
  for (struct ifaddrs *ifa = ifAddrList; ifa != nullptr; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr) continue;
    if (ifa->ifa_addr->sa_family != AF_INET) continue;

    // Skip loopback
    if (ifa->ifa_flags & IFF_LOOPBACK) continue;
    // Only consider interfaces that are UP
    if (!(ifa->ifa_flags & IFF_UP)) continue;

    auto *sa = reinterpret_cast<sockaddr_in *>(ifa->ifa_addr);
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &sa->sin_addr, ipStr, sizeof(ipStr));
    result = std::string(ipStr);
    break; // Take the first non-loopback IPv4 address
  }

  freeifaddrs(ifAddrList);
  return result;
#endif
}

// ── Private helpers ─────────────────────────────────────────────────────────

void NetworkSession::receiveLoop() {
  constexpr size_t kBufferSize = 1024;
  uint8_t recvBuf[kBufferSize];

  // Accumulation buffer for partial messages
  std::vector<uint8_t> accumulator;

  while (!shouldStop_) {
    // Blocking recv — will be unblocked by shutdown() in disconnect()
#ifdef _WIN32
    int bytesRead = recv(peerSocket_, reinterpret_cast<char *>(recvBuf),
                         static_cast<int>(kBufferSize), 0);
#else
    ssize_t bytesRead = recv(peerSocket_, recvBuf, kBufferSize, 0);
#endif

    if (bytesRead <= 0) {
      // Connection closed or error
      if (!shouldStop_) {
        std::cerr << "[NetworkSession] Connection lost (recv returned "
                  << bytesRead << ")\n";
        connected_ = false;
      }
      break;
    }

    // Append received data to accumulation buffer
    accumulator.insert(accumulator.end(), recvBuf, recvBuf + bytesRead);

    // Decode as many complete messages as possible
    while (!accumulator.empty()) {
      size_t consumed = 0;
      auto msg = Protocol::decodeMessage(accumulator.data(),
                                         accumulator.size(), consumed);
      if (!msg) break; // Incomplete message — wait for more data

      incomingQueue_.push(std::move(*msg));
      accumulator.erase(accumulator.begin(),
                        accumulator.begin() +
                            static_cast<std::ptrdiff_t>(consumed));
    }
  }
}

bool NetworkSession::sendRaw(const std::vector<uint8_t> &data) {
  if (peerSocket_ == kInvalidSocket || !connected_) return false;

  size_t totalSent = 0;
  while (totalSent < data.size()) {
    const uint8_t *ptr = data.data() + totalSent;
    size_t remaining = data.size() - totalSent;

#ifdef _WIN32
    int sent = send(peerSocket_, reinterpret_cast<const char *>(ptr),
                    static_cast<int>(remaining), 0);
#else
    ssize_t sent = send(peerSocket_, ptr, remaining, MSG_NOSIGNAL);
#endif

    if (sent <= 0) {
      connected_ = false;
      return false;
    }
    totalSent += static_cast<size_t>(sent);
  }
  return true;
}

void NetworkSession::closeAndReset(SocketHandle &sock) {
  if (sock != kInvalidSocket) {
    closeSocket(sock);
    sock = kInvalidSocket;
  }
}
