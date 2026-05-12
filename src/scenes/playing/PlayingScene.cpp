#include "scenes/playing/PlayingScene.h"

#include "scenes/main_menu/MenuModel.h"

PlayingScene::PlayingScene(PlayMode mode, bool loadSave)
    : mode_(mode), shouldLoadSave_(loadSave) {}

PlayingScene::~PlayingScene() {
  if (game_ != nullptr) {
    const std::string saveFile =
        (mode_ == PlayMode::PvP) ? "save_pvp.bin" : "save_pve.bin";
    game_->saveGame(saveFile);
  }
}

void PlayingScene::update(SceneManager *manager) {
  if (!initialized_) {
    const std::string saveFile =
        (mode_ == PlayMode::PvP) ? "save_pvp.bin" : "save_pve.bin";

    game_ = std::make_unique<Game>();
    game_->setTimeControl(g_targetTimeControl);
    view_ = std::make_unique<ChessView>();
    sound_ = std::make_unique<ChessSound>();
    sound_->loadSounds();

    game_->attach(view_.get());
    game_->attach(sound_.get());

    if (!view_->LoadAssets()) {
      manager->changeScene(SceneType::MAIN_MENU);
      initialized_ = true;
      return;
    }

    controller_ = std::make_unique<ChessController>(*game_, *view_, saveFile);

    if (shouldLoadSave_ && !game_->loadGame(saveFile)) {
      loadFailed_ = true;
      loadFailStartTime_ = GetTime();
      loadFailMessage_ = "Failed to load " + saveFile + " - returning to menu";
    }

    initialized_ = true;
  }

  if (loadFailed_) {
    if (GetTime() - loadFailStartTime_ >= kLoadFailMessageDurationSeconds_) {
      manager->changeScene(SceneType::MAIN_MENU);
    }
    return;
  }

  if (controller_ != nullptr && controller_->processInput()) {
    manager->changeScene(SceneType::MAIN_MENU);
  }
}

void PlayingScene::render() {
  if (!initialized_) {
    ClearBackground(BLACK);
    const char *txt = "Starting game...";
    DrawText(txt, GetScreenWidth() / 2 - MeasureText(txt, 20) / 2,
             GetScreenHeight() / 2 - 10, 20, WHITE);
    return;
  }

  if (loadFailed_) {
    ClearBackground({30, 30, 30, 255});
    DrawText(loadFailMessage_.c_str(),
             GetScreenWidth() / 2 - MeasureText(loadFailMessage_.c_str(), 20) / 2,
             GetScreenHeight() / 2 - 10, 20, RAYWHITE);
    return;
  }

  if (controller_ != nullptr) {
    controller_->render();
  }
}
