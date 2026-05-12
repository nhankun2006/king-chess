#include "scenes/playing/states/DraggingInteractionState.h"

#include "scenes/playing/ChessController.h"
#include "scenes/playing/states/IdleInteractionState.h"
#include "scenes/playing/states/PromotionInteractionState.h"

#include <raylib.h>

DraggingInteractionState::DraggingInteractionState(Position from,
                                                   PieceType type,
                                                   ChessColor color)
    : dragFrom_(from), dragPieceType_(type), dragPieceColor_(color) {}

bool DraggingInteractionState::handleInput(ChessController &ctrl) {
  if (!IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    return false;
  }

  const Vector2 mousePos = GetMousePosition();
  Position dropSquare{};
  const bool hasDropSquare =
      ctrl.view_->screenToBoardSquare(mousePos.x, mousePos.y, dropSquare);

  bool movedThisFrame = false;

  if (hasDropSquare && ctrl.selectedSquare_.has_value()) {
    const Position from = ctrl.selectedSquare_.value();

    // Delegate rule-level queries to the Game model
    if (ctrl.game_->hasPromotionChoices(from, dropSquare)) {
      // Transition to promotion dialog
      ctrl.setState(std::make_unique<PromotionInteractionState>(
          from, dropSquare, dragPieceColor_));
      return false;
    }

    auto move = ctrl.game_->resolveLegalMove(from, dropSquare);
    if (move.has_value()) {
      movedThisFrame = ctrl.applyMove(move.value());
    }
  }

  // Invalid drop feedback
  if (!movedThisFrame && hasDropSquare && ctrl.selectedSquare_.has_value() &&
      !(dropSquare == ctrl.selectedSquare_.value())) {
    ctrl.triggerInvalidMoveWarning(dropSquare);
  }

  ctrl.clearSelection();
  ctrl.setState(std::make_unique<IdleInteractionState>());
  return false;
}

std::optional<DragPreview> DraggingInteractionState::getDragPreview() const {
  DragPreview dp;
  dp.type = dragPieceType_;
  dp.color = dragPieceColor_;
  dp.from = dragFrom_;
  dp.mousePos = GetMousePosition();
  return dp;
}
