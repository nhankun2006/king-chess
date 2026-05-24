#ifndef NETWORKSESSION_H
#define NETWORKSESSION_H

#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include "chess/model/Move.h"
#include "chess/net/Platform.h"
#include "chess/net/Protocol.h"
#include "chess/net/ThreadSafeQueue.h"

/// Manages a single TCP connection between two chess peers (host ↔ guest).
///
/// Threading model:
///   - The main thread calls sendMove() / sendMessage() / tryReceive().
///   - A dedicated background thread runs receiveLoop(), reading from the
///     socket and pushing decoded messages into incomingQueue_.
///   - The Game model is never accessed from the network thread.
///
/// Lifecycle:
///   1. Create a NetworkSession.
///   2. Call hostAndWaitForClient() OR connectToHost() (blocking).
///   3. Use sendMove() / tryReceive() during gameplay (non-blocking).
///   4. Call disconnect() or let the destructor clean up.
class NetworkSession {
public:
  NetworkSession();
  ~NetworkSession();

  // Non-copyable, non-movable (owns a thread)
  NetworkSession(const NetworkSession &) = delete;
  NetworkSession &operator=(const NetworkSession &) = delete;

  /// Default port for LAN chess.
  static constexpr int kDefaultPort = 7777;

  // ── Connection lifecycle (blocking — call from lobby, not game loop) ──

  /// Host: bind + listen + accept one client.  Blocks until a client connects.
  /// Returns true on success.
  bool hostAndWaitForClient(int port = kDefaultPort);

  /// Guest: connect to a host at the given IP and port.  Blocks until
  /// the connection is established or fails.  Returns true on success.
  bool connectToHost(const std::string &ip, int port = kDefaultPort);

  // ── Game communication (non-blocking — call from the game loop) ────────

  /// Send a chess move to the remote peer.
  void sendMove(const Move &move);

  /// Send a control message (Restart, Resign, Ping, etc.).
  void sendMessage(Protocol::MessageType type);

  /// Poll for the next incoming message.
  /// Returns std::nullopt if no complete message is available yet.
  std::optional<Protocol::Message> tryReceive();

  /// Is the TCP connection currently alive?
  bool isConnected() const;

  /// Gracefully shut down the connection and join the receive thread.
  void disconnect();

  /// Get the local IP address (useful for the host to display to the guest).
  static std::string getLocalIPAddress();

private:
  SocketHandle serverSocket_ = kInvalidSocket; // only used by host
  SocketHandle peerSocket_ = kInvalidSocket;   // the active connection

  std::thread receiveThread_;
  std::atomic<bool> connected_{false};
  std::atomic<bool> shouldStop_{false};

  ThreadSafeQueue<Protocol::Message> incomingQueue_;

  /// Background thread entry point.  Reads raw bytes from the socket,
  /// accumulates them in a buffer, and decodes complete Protocol::Messages
  /// which are pushed into incomingQueue_.
  void receiveLoop();

  /// Send raw bytes over the peer socket.  Returns true on success.
  bool sendRaw(const std::vector<uint8_t> &data);

  /// Close a socket handle and reset it to kInvalidSocket.
  void closeAndReset(SocketHandle &sock);
};

#endif // NETWORKSESSION_H
