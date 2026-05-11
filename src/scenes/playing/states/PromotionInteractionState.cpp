#include "scenes/playing/states/PromotionInteractionState.h"

#include "scenes/playing/ChessController.h"
#include "scenes/playing/states/IdleInteractionState.h"

#include <raylib.h>

PromotionInteractionState::PromotionInteractionState(
    std::vector<Move> promotionMoves, ChessColor color)
    : pendingMoves_(std::move(promotionMoves)), color_(color) {}

bool PromotionInteractionState::handleInput(ChessController &ctrl) {
  if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    return false;
  }

  const Vector2 mousePos = GetMousePosition();
  const PieceType selectedPromotion =
      ctrl.view_->getPromotionOptionClicked(mousePos.x, mousePos.y);

  if (selectedPromotion != PieceType::None) {
    for (const auto &move : pendingMoves_) {
      if (move.promotion != selectedPromotion) {
        continue;
      }
      if (ctrl.applyMove(move)) {
        ctrl.promotionPromptOpen_ = false;
        ctrl.clearSelection();
        ctrl.stopDragging();
        ctrl.setState(std::make_unique<IdleInteractionState>());
        return false;
      }
    }
  } else {
    ctrl.triggerInvalidMoveWarning(std::nullopt);
  }

  return false;
}
