#include "scenes/playing/states/RestartModalInteractionState.h"

#include "scenes/playing/ChessController.h"
#include "scenes/playing/states/IdleInteractionState.h"

#include <raylib.h>

bool RestartModalInteractionState::handleInput(ChessController &ctrl) {
  if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    return false;
  }

  const Vector2 mousePos = GetMousePosition();

  if (ctrl.view_->isRestartConfirmYesClicked(mousePos.x, mousePos.y)) {
    ctrl.game_->restart();
    ctrl.view_->resetVisualEffects();
    ctrl.clearSelection();
    ctrl.stopDragging();
    ctrl.restartConfirmOpen_ = false;
    ctrl.setState(std::make_unique<IdleInteractionState>());
  } else if (ctrl.view_->isRestartConfirmNoClicked(mousePos.x, mousePos.y)) {
    ctrl.restartConfirmOpen_ = false;
    ctrl.setState(std::make_unique<IdleInteractionState>());
  }

  return false;
}
