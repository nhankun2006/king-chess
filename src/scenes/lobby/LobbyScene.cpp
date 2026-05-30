#include "scenes/lobby/LobbyScene.h"

#include <raylib.h>

#include "config/UIConfig.h"
#include "scenes/playing/PlayingScene.h"
#include "ui/Button.h"

// ── Construction / Destruction ───────────────────────────────────────────────

LobbyScene::LobbyScene() {
  session_ = std::make_shared<NetworkSession>();
}

LobbyScene::~LobbyScene() {
  // Ensure the background thread is finished before destroying
  joinThread();
  // If we still own the session and it's connected, disconnect
  if (session_) {
    session_->disconnect();
  }
}

// ── Thread management ────────────────────────────────────────────────────────

void LobbyScene::joinThread() {
  if (!threadJoined_ && connectThread_.joinable()) {
    connectThread_.join();
    threadJoined_ = true;
  }
}

void LobbyScene::startHosting() {
  localIp_ = NetworkSession::getLocalIPAddress();
  statusMessage_ = "Waiting for opponent...";
  connectDone_ = false;
  connectResult_ = false;
  threadJoined_ = false;

  // Launch accept() on a background thread so the UI doesn't freeze
  auto sessionPtr = session_;
  connectThread_ = std::thread([sessionPtr]() {
    bool ok = sessionPtr->hostAndWaitForClient(7777);
    // The atomic flags are set in update() by checking isConnected()
    (void)ok;
  });

  state_ = LobbyState::HOSTING;
}

void LobbyScene::startConnecting() {
  if (ipInput_.empty()) {
    statusMessage_ = "Please enter an IP address";
    return;
  }

  statusMessage_ = "Connecting to " + ipInput_ + "...";
  connectDone_ = false;
  connectResult_ = false;
  threadJoined_ = false;

  auto sessionPtr = session_;
  std::string ip = ipInput_;
  connectThread_ = std::thread([sessionPtr, ip]() {
    bool ok = sessionPtr->connectToHost(ip, 7777);
    (void)ok;
  });

  state_ = LobbyState::CONNECTING;
}

// ── Update ───────────────────────────────────────────────────────────────────

void LobbyScene::update(SceneManager *manager) {
  ui::AutoLayout::Metrics metrics =
      ui::AutoLayout::ComputeMetrics(GetScreenWidth(), GetScreenHeight());
  float scale = metrics.uiScale;
  float centerX = GetScreenWidth() / 2.0f;
  float centerY = GetScreenHeight() / 2.0f;
  float btnWidth = 240.0f * scale;
  float btnHeight = 56.0f * scale;
  float btnGap = 80.0f * scale;

  Vector2 mousePos = GetMousePosition();
  bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

  auto makeButtonBounds = [&](int idx) -> Rectangle {
    float y = centerY + idx * btnGap - btnHeight * 0.8f + 50.0f * (btnHeight / 56.0f);
    return {centerX - btnWidth / 2.0f, y, btnWidth, btnHeight};
  };

  Rectangle backBounds = {20.0f * scale,
                          GetScreenHeight() - 40.0f * scale - 20.0f * scale,
                          80.0f * scale, 40.0f * scale};

  switch (state_) {
  case LobbyState::CHOOSE_ROLE: {
    Rectangle hostBtn = makeButtonBounds(0);
    Rectangle joinBtn = makeButtonBounds(1);

    if (clicked) {
      if (CheckCollisionPointRec(mousePos, hostBtn)) {
        startHosting();
      } else if (CheckCollisionPointRec(mousePos, joinBtn)) {
        state_ = LobbyState::JOINING;
        ipInput_.clear();
        statusMessage_.clear();
      } else if (CheckCollisionPointRec(mousePos, backBounds)) {
        manager->changeScene(SceneType::MAIN_MENU);
      }
    }
    break;
  }

  case LobbyState::HOSTING: {
    // Poll: did a client connect?
    if (session_->isConnected()) {
      joinThread();
      state_ = LobbyState::CONNECTED;
    }

    // Cancel button
    if (clicked && CheckCollisionPointRec(mousePos, backBounds)) {
      session_->disconnect();
      joinThread();
      session_ = std::make_shared<NetworkSession>();
      state_ = LobbyState::CHOOSE_ROLE;
    }
    break;
  }

  case LobbyState::JOINING: {
    // Handle keyboard input for IP address
    int key = GetCharPressed();
    while (key > 0) {
      // Only allow digits and dots
      if ((key >= '0' && key <= '9') || key == '.') {
        if (static_cast<int>(ipInput_.size()) < kMaxIpLength) {
          ipInput_ += static_cast<char>(key);
        }
      }
      key = GetCharPressed();
    }

    // Backspace
    if (IsKeyPressed(KEY_BACKSPACE) && !ipInput_.empty()) {
      ipInput_.pop_back();
    }

    // Enter = connect
    if (IsKeyPressed(KEY_ENTER)) {
      startConnecting();
      break;
    }

    // Connect button
    Rectangle connectBtn = makeButtonBounds(1);
    if (clicked && CheckCollisionPointRec(mousePos, connectBtn)) {
      startConnecting();
      break;
    }

    // Back button
    if (clicked && CheckCollisionPointRec(mousePos, backBounds)) {
      state_ = LobbyState::CHOOSE_ROLE;
    }
    break;
  }

  case LobbyState::CONNECTING: {
    // Poll: did the connection succeed?
    if (session_->isConnected()) {
      joinThread();
      state_ = LobbyState::CONNECTED;
    }

    // Check if the thread has finished (connect failed)
    if (!threadJoined_) {
      // Non-blocking check: try_join_for would be nicer but std::thread
      // doesn't have it. We check if the session is connected OR if
      // the socket is invalid (failed).
      if (!session_->isConnected()) {
        // The thread might still be running if connect() is blocking...
        // We'll let it finish naturally. Check if thread is joinable
        // and the connect has returned by a simple heuristic:
        // If the thread hasn't made us connected after it started,
        // and the socket operation should complete quickly (connect timeout),
        // we poll with a short wait.
      }
    }

    // Cancel
    if (clicked && CheckCollisionPointRec(mousePos, backBounds)) {
      session_->disconnect();
      joinThread();
      session_ = std::make_shared<NetworkSession>();
      state_ = LobbyState::JOINING;
    }
    break;
  }

  case LobbyState::CONNECTED: {
    // Auto-transition to PlayingScene
    // Determine if we are host or guest based on who initiated
    // Host: we called hostAndWaitForClient → we play White
    // Guest: we called connectToHost → we play Black
    bool isHost = localIp_.size() > 0; // Host flow sets localIp_

    PlayMode mode = isHost ? PlayMode::LAN_Host : PlayMode::LAN_Guest;

    auto playingScene = std::make_unique<PlayingScene>(mode, false, session_);
    // Transfer ownership: PlayingScene now co-owns the session
    session_.reset();
    manager->changeSceneWithInstance(std::move(playingScene));
    break;
  }

  case LobbyState::ERROR: {
    if (clicked && CheckCollisionPointRec(mousePos, backBounds)) {
      session_ = std::make_shared<NetworkSession>();
      state_ = LobbyState::CHOOSE_ROLE;
      statusMessage_.clear();
    }
    break;
  }
  }
}

// ── Render ───────────────────────────────────────────────────────────────────

void LobbyScene::render() {
  ClearBackground({26, 31, 41, 255});

  ui::AutoLayout::Metrics metrics =
      ui::AutoLayout::ComputeMetrics(GetScreenWidth(), GetScreenHeight());
  float scale = metrics.uiScale;
  float centerX = GetScreenWidth() / 2.0f;
  float centerY = GetScreenHeight() / 2.0f;
  float btnWidth = 240.0f * scale;
  float btnHeight = 56.0f * scale;
  float btnGap = 80.0f * scale;

  int titleFont = static_cast<int>(36 * scale);
  int subtitleFont = static_cast<int>(20 * scale);
  int bodyFont = static_cast<int>(18 * scale);

  auto makeButtonBounds = [&](int idx) -> Rectangle {
    float y = centerY + idx * btnGap - btnHeight * 0.8f + 50.0f * (btnHeight / 56.0f);
    return {centerX - btnWidth / 2.0f, y, btnWidth, btnHeight};
  };

  auto drawMenuBtn = [&](Rectangle bounds, const char *text) {
    ui::ButtonStyle style;
    style.fontSize = static_cast<int>(26 * scale);
    style.baseColor = ui::IconButtons::kButtonFill;
    style.hoverColor = ui::IconButtons::kButtonFillHover;
    style.borderColor = ui::IconButtons::kButtonBorder;
    style.borderHoverColor = ui::IconButtons::kButtonBorderHover;
    style.textColor = ui::Dialog::kTextPrimary;
    style.roundness = ui::IconButtons::kRoundness;
    style.segments = ui::IconButtons::kSegments;
    style.borderWidth = ui::IconButtons::kBorderWidth;

    ui::Button btn(bounds, text, style);
    btn.update(GetMousePosition());
    btn.draw();
  };

  auto drawBackBtn = [&]() {
    Rectangle bounds = {20.0f * scale,
                        GetScreenHeight() - 40.0f * scale - 20.0f * scale,
                        80.0f * scale, 40.0f * scale};
    ui::ButtonStyle style;
    style.baseColor = ui::IconButtons::kButtonFill;
    style.hoverColor = ui::IconButtons::kButtonFillHover;
    style.borderColor = ui::IconButtons::kButtonBorder;
    style.borderHoverColor = ui::IconButtons::kButtonBorderHover;
    style.textColor = ui::Dialog::kTextPrimary;
    style.fontSize = static_cast<int>(20 * scale);

    ui::Button btn(bounds, "<-", style);
    btn.update(GetMousePosition());
    btn.draw();
  };

  // Title
  const char *title = "LAN Multiplayer";
  DrawText(title,
           static_cast<int>(centerX - MeasureText(title, titleFont) / 2),
           static_cast<int>(centerY - 150.0f * scale), titleFont,
           ui::Dialog::kTextPrimary);

  switch (state_) {
  case LobbyState::CHOOSE_ROLE: {
    drawMenuBtn(makeButtonBounds(0), "Host Game");
    drawMenuBtn(makeButtonBounds(1), "Join Game");
    drawBackBtn();
    break;
  }

  case LobbyState::HOSTING: {
    // Show local IP prominently
    const char *ipLabel = "Your IP Address:";
    DrawText(ipLabel,
             static_cast<int>(centerX - MeasureText(ipLabel, subtitleFont) / 2),
             static_cast<int>(centerY - 20.0f * scale), subtitleFont,
             ui::Dialog::kTextSecondary);

    int ipFont = static_cast<int>(32 * scale);
    DrawText(localIp_.c_str(),
             static_cast<int>(centerX - MeasureText(localIp_.c_str(), ipFont) / 2),
             static_cast<int>(centerY + 20.0f * scale), ipFont,
             {110, 200, 140, 255}); // Green accent for IP

    DrawText("Port: 7777",
             static_cast<int>(centerX - MeasureText("Port: 7777", bodyFont) / 2),
             static_cast<int>(centerY + 65.0f * scale), bodyFont,
             ui::Dialog::kTextSecondary);

    // Waiting indicator with animated dots
    int dotCount = static_cast<int>(GetTime() * 2.0) % 4;
    std::string waitText = "Waiting for opponent";
    for (int i = 0; i < dotCount; ++i) waitText += '.';

    DrawText(waitText.c_str(),
             static_cast<int>(centerX - MeasureText(waitText.c_str(), bodyFont) / 2),
             static_cast<int>(centerY + 110.0f * scale), bodyFont,
             {180, 190, 210, 255});

    drawBackBtn();
    break;
  }

  case LobbyState::JOINING: {
    // IP input label
    const char *ipLabel = "Enter Host IP:";
    DrawText(ipLabel,
             static_cast<int>(centerX - MeasureText(ipLabel, subtitleFont) / 2),
             static_cast<int>(centerY - 30.0f * scale), subtitleFont,
             ui::Dialog::kTextSecondary);

    // Input field background
    float fieldW = 260.0f * scale;
    float fieldH = 44.0f * scale;
    Rectangle fieldRect = {centerX - fieldW / 2.0f,
                           centerY + 10.0f * scale, fieldW, fieldH};
    DrawRectangleRounded(fieldRect, 0.2f, 8, {40, 48, 65, 255});
    DrawRectangleRoundedLinesEx(fieldRect, 0.2f, 8, 2.0f, {90, 105, 130, 255});

    // Cursor blink
    bool showCursor = (static_cast<int>(GetTime() * 2.0f) % 2 == 0);
    std::string displayText = ipInput_ + (showCursor ? "|" : "");

    int inputFont = static_cast<int>(24 * scale);
    DrawText(displayText.c_str(),
             static_cast<int>(fieldRect.x + 12.0f * scale),
             static_cast<int>(fieldRect.y + (fieldH - inputFont) / 2),
             inputFont, ui::Dialog::kTextPrimary);

    // Connect button
    drawMenuBtn(makeButtonBounds(1), "Connect");

    // Status message
    if (!statusMessage_.empty()) {
      DrawText(statusMessage_.c_str(),
               static_cast<int>(centerX - MeasureText(statusMessage_.c_str(), bodyFont) / 2),
               static_cast<int>(centerY + 130.0f * scale), bodyFont,
               {220, 100, 100, 255});
    }

    drawBackBtn();
    break;
  }

  case LobbyState::CONNECTING: {
    int dotCount = static_cast<int>(GetTime() * 2.0) % 4;
    std::string connectText = "Connecting";
    for (int i = 0; i < dotCount; ++i) connectText += '.';

    DrawText(connectText.c_str(),
             static_cast<int>(centerX - MeasureText(connectText.c_str(), subtitleFont) / 2),
             static_cast<int>(centerY + 20.0f * scale), subtitleFont,
             {180, 190, 210, 255});

    DrawText(ipInput_.c_str(),
             static_cast<int>(centerX - MeasureText(ipInput_.c_str(), bodyFont) / 2),
             static_cast<int>(centerY + 60.0f * scale), bodyFont,
             {110, 200, 140, 255});

    drawBackBtn();
    break;
  }

  case LobbyState::CONNECTED: {
    const char *msg = "Connected! Starting game...";
    DrawText(msg,
             static_cast<int>(centerX - MeasureText(msg, subtitleFont) / 2),
             static_cast<int>(centerY + 20.0f * scale), subtitleFont,
             {110, 200, 140, 255});
    break;
  }

  case LobbyState::ERROR: {
    DrawText(statusMessage_.c_str(),
             static_cast<int>(centerX - MeasureText(statusMessage_.c_str(), subtitleFont) / 2),
             static_cast<int>(centerY + 20.0f * scale), subtitleFont,
             {220, 100, 100, 255});

    const char *hint = "Press Back to return";
    DrawText(hint,
             static_cast<int>(centerX - MeasureText(hint, bodyFont) / 2),
             static_cast<int>(centerY + 60.0f * scale), bodyFont,
             ui::Dialog::kTextSecondary);

    drawBackBtn();
    break;
  }
  }
}
