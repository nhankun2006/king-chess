#include "scenes/playing/states/BotTurnState.h"

#include "scenes/playing/ChessController.h"
#include "scenes/playing/states/IdleInteractionState.h"

#include <chrono>

bool BotTurnState::handleInput(ChessController &ctrl) {
  // First frame: launch the AI computation on a background thread
  if (!launched_) {
    IPlayerAgent *agent =
        (ctrl.game_->getCurrentTurn() == ChessColor::White)
            ? ctrl.whitePlayer_.get()
            : ctrl.blackPlayer_.get();

    if (agent == nullptr || !agent->isAutomated()) {
      ctrl.setState(std::make_unique<IdleInteractionState>());
      return false;
    }

    Game *game = ctrl.game_;
    future_ = std::async(std::launch::async,
                         [agent, game]() { return agent->chooseMove(*game); });
    launched_ = true;
    return false;
  }

  // Subsequent frames: poll the future without blocking
  if (future_.wait_for(std::chrono::milliseconds(0)) ==
      std::future_status::ready) {
    auto optMove = future_.get();
    if (optMove.has_value()) {
      ctrl.applyMove(optMove.value());
    }
    ctrl.setState(std::make_unique<IdleInteractionState>());
  }

  return false;
}
