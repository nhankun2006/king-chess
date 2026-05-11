#include "scenes/playing/PlayingScene.h"

#include "chess/players/PlayerFactory.h"

PlayingScene::PlayingScene(PlayMode mode, bool loadSave)
    : mode_(mode), shouldLoadSave_(loadSave) {}

PlayingScene::~PlayingScene() {
  if (game_ != nullptr) {
    game_->saveGame("save.bin");
  }
}

void PlayingScene::update(SceneManager *manager) {
  if (!initialized_) {
    game_ = std::make_unique<Game>();
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

    auto players = PlayerFactory::create(mode_);
    controller_ = std::make_unique<ChessController>(
        *game_, *view_, std::move(players.white), std::move(players.black));

    if (shouldLoadSave_ && !game_->loadGame("save.bin")) {
      loadFailed_ = true;
      loadFailStartTime_ = GetTime();
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
    DrawText(kLoadFailMessage_,
             GetScreenWidth() / 2 - MeasureText(kLoadFailMessage_, 20) / 2,
             GetScreenHeight() / 2 - 10, 20, RAYWHITE);
    return;
  }

  if (controller_ == nullptr || game_ == nullptr || view_ == nullptr) {
    return;
  }

  ChessColor winnerColorVal = ChessColor::White;
  ChessColor *winnerColor = nullptr;
  const GameState gameState = game_->getState();
  if (gameState == GameState::Checkmate) {
    winnerColorVal = oppositeColor(game_->getCurrentTurn());
    winnerColor = &winnerColorVal;
  }

  const ChessController::RenderState renderState = controller_->buildRenderState();
  const DragPreview *dragPreview =
      renderState.dragPreview.has_value() ? &renderState.dragPreview.value()
                                          : nullptr;
  const ChessColor *promotionColor =
      renderState.promotionColor.has_value() ? &renderState.promotionColor.value()
                                             : nullptr;

  const Position *selectedSquare = nullptr;
  if (controller_->getSelectedSquare().has_value()) {
    selectedSquare = &controller_->getSelectedSquare().value();
  }

  view_->drawBoard(game_->getBoard(), selectedSquare, controller_->getLegalMoves(),
                   renderState.showRestartConfirm,
                   renderState.showWindowSizeDialog, gameState, winnerColor,
                   dragPreview, promotionColor, renderState.showSaveMessage);
}
