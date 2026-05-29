#include "scenes/playing/states/NetworkWaitState.h"

#include "scenes/playing/ChessController.h"
#include "chess/net/NetworkSession.h"
#include "scenes/playing/states/IdleInteractionState.h"

bool NetworkWaitState::handleInput(ChessController &ctrl) {
  // If the game is over, return to idle (no more moves expected)
  if (!ctrl.gameIsPlayable()) {
    ctrl.setState(std::make_unique<IdleInteractionState>());
    return false;
  }

  // Check if the remote peer has disconnected
  if (ctrl.networkSession_ && !ctrl.networkSession_->isConnected()) {
    // TODO Phase 4: show a "Disconnected" dialog and return to menu.
    // For now, transition to idle so the UI doesn't freeze.
    ctrl.setState(std::make_unique<IdleInteractionState>());
    return false;
  }

  // Poll the NetworkPlayerAgent for a move
  IPlayerAgent *agent =
      (ctrl.game_->getCurrentTurn() == ChessColor::White)
          ? ctrl.whitePlayer_.get()
          : ctrl.blackPlayer_.get();

  if (agent == nullptr) {
    ctrl.setState(std::make_unique<IdleInteractionState>());
    return false;
  }

  auto optMove = agent->chooseMove(*ctrl.game_);
  if (optMove.has_value()) {
    ctrl.applyMove(optMove.value());
    ctrl.setState(std::make_unique<IdleInteractionState>());
  }

  return false;
}
