#include "scenes/playing/states/PromotionInteractionState.h"

#include "scenes/playing/ChessController.h"
#include "scenes/playing/states/IdleInteractionState.h"

#include <raylib.h>

PromotionInteractionState::PromotionInteractionState(Position from, Position to,
                                                     ChessColor color)
    : from_(from), to_(to), color_(color) {}

bool PromotionInteractionState::handleInput(ChessController &ctrl) {
  if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    return false;
  }

  const Vector2 mousePos = GetMousePosition();
  const PieceType selectedPromotion =
      ctrl.view_->getPromotionOptionClicked(mousePos.x, mousePos.y);

  if (selectedPromotion != PieceType::None) {
    // Ask the Game model to resolve the exact move — no manual filtering
    auto move = ctrl.game_->resolveLegalMove(from_, to_, selectedPromotion);
    if (move.has_value() && ctrl.applyMove(move.value())) {
      ctrl.clearSelection();
      ctrl.setState(std::make_unique<IdleInteractionState>());
      return false;
    }
  } else {
    ctrl.triggerInvalidMoveWarning(std::nullopt);
  }

  return false;
}
