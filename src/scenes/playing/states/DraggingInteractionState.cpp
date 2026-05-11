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

  const Piece *selectedPiece =
      ctrl.selectedSquare_.has_value()
          ? ctrl.game_->getBoard().getPieceAt(ctrl.selectedSquare_.value())
          : nullptr;

  bool movedThisFrame = false;

  if (hasDropSquare && selectedPiece != nullptr) {
    std::vector<Move> candidateMoves;
    for (const auto &legalMove : ctrl.selectedLegalMoves_) {
      if (legalMove.to == dropSquare) {
        candidateMoves.push_back(legalMove);
      }
    }

    if (!candidateMoves.empty()) {
      // Non-promotion move (single candidate or non-pawn)
      if (candidateMoves.size() == 1 ||
          selectedPiece->getType() != PieceType::Pawn) {
        movedThisFrame = ctrl.applyMove(candidateMoves.front());
      } else {
        // Check whether the candidates include promotions
        std::vector<Move> promotionMoves;
        for (const auto &move : candidateMoves) {
          if (move.promotion != PieceType::None) {
            promotionMoves.push_back(move);
          }
        }

        if (!promotionMoves.empty()) {
          // Transition to promotion dialog
          ctrl.promotionPromptOpen_ = true;
          ctrl.promotionPromptColor_ = selectedPiece->getColor();
          ctrl.stopDragging();
          ctrl.setState(std::make_unique<PromotionInteractionState>(
              std::move(promotionMoves), selectedPiece->getColor()));
          return false;
        }
        // Multiple non-promotion candidates (shouldn't normally happen)
        movedThisFrame = ctrl.applyMove(candidateMoves.front());
      }
    }
  }

  // Invalid drop feedback
  if (!movedThisFrame && hasDropSquare && ctrl.selectedSquare_.has_value() &&
      !(dropSquare == ctrl.selectedSquare_.value())) {
    ctrl.triggerInvalidMoveWarning(dropSquare);
  }

  ctrl.clearSelection();
  ctrl.stopDragging();
  ctrl.setState(std::make_unique<IdleInteractionState>());
  return false;
}
