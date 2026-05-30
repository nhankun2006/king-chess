#ifndef LOBBYSCENE_H
#define LOBBYSCENE_H

#include "core/Scene.h"
#include "core/SceneManager.h"

#include <memory>
#include <string>
#include <thread>
#include <atomic>

#include "chess/net/NetworkSession.h"
#include "scenes/PlayMode.h"

/// The Lobby scene handles host/join selection, connection setup, and
/// transitions to PlayingScene once a TCP connection is established.
///
/// Flow:
///   CHOOSE_ROLE  → user picks Host or Join
///   HOSTING      → listening on a port, waiting for a guest (background thread)
///   JOINING      → user enters IP, clicks Connect (background thread)
///   CONNECTED    → connection established → auto-transition to PlayingScene
///   ERROR        → connection failed → show message, allow retry
class LobbyScene : public Scene {
public:
  LobbyScene();
  ~LobbyScene() override;

  void update(SceneManager *manager) override;
  void render() override;

private:
  enum class LobbyState {
    CHOOSE_ROLE,
    HOSTING,
    JOINING,
    CONNECTING,
    CONNECTED,
    ERROR
  };

  LobbyState state_ = LobbyState::CHOOSE_ROLE;

  // The network session — owned here, transferred to PlayingScene on success
  std::shared_ptr<NetworkSession> session_;

  // Background thread for blocking connect/accept
  std::thread connectThread_;
  std::atomic<bool> connectResult_{false};
  std::atomic<bool> connectDone_{false};

  // IP input for the Join flow
  std::string ipInput_ = "";
  static constexpr int kMaxIpLength = 15; // "xxx.xxx.xxx.xxx"

  // Cached local IP for the Host flow
  std::string localIp_;

  // Error/status message
  std::string statusMessage_;

  // Whether the connection thread has been joined
  bool threadJoined_ = true;

  void startHosting();
  void startConnecting();
  void joinThread();
};

#endif // LOBBYSCENE_H
