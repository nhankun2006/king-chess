#include "scenes/playing/states/IdleInteractionState.h"

#include "scenes/playing/ChessController.h"
#include "scenes/playing/states/DraggingInteractionState.h"
#include "scenes/playing/states/RestartModalInteractionState.h"
#include "scenes/playing/states/WindowModalInteractionState.h"

#include <raylib.h>

bool IdleInteractionState::handleInput(ChessController &ctrl) {
  if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    return false;
  }

  const Vector2 mousePos = GetMousePosition();

  // ── Toolbar buttons ──────────────────────────────────────────────────────
  if (ctrl.view_->isSettingsButtonClicked(mousePos.x, mousePos.y)) {
    ctrl.clearSelection();
    ctrl.windowSizeDialogOpen_ = true;
    ctrl.setState(std::make_unique<WindowModalInteractionState>());
    return false;
  }

  if (ctrl.view_->isRotateButtonClicked(mousePos.x, mousePos.y)) {
    ctrl.clearSelection();
    ctrl.view_->toggleBoardOrientation();
    return false;
  }

  if (ctrl.view_->isRestartButtonClicked(mousePos.x, mousePos.y)) {
    ctrl.clearSelection();
    ctrl.windowSizeDialogOpen_ = false;
    ctrl.restartConfirmOpen_ = true;
    ctrl.setState(std::make_unique<RestartModalInteractionState>());
    return false;
  }

  if (ctrl.view_->isUndoButtonClicked(mousePos.x, mousePos.y)) {
    ctrl.clearSelection();
    ctrl.windowSizeDialogOpen_ = false;
    ctrl.undoForCurrentMode();
    return false;
  }

  // ── Board click — start dragging a piece ─────────────────────────────────
  Position clickedSquare;
  if (ctrl.view_->screenToBoardSquare(mousePos.x, mousePos.y, clickedSquare)) {
    const Piece *clickedPiece =
        ctrl.game_->getBoard().getPieceAt(clickedSquare);
    if (clickedPiece != nullptr &&
        clickedPiece->getColor() == ctrl.game_->getCurrentTurn()) {
      ctrl.updateSelection(clickedSquare);
      ctrl.isDraggingPiece_ = true;
      ctrl.dragFromSquare_ = clickedSquare;
      ctrl.dragPieceType_ = clickedPiece->getType();
      ctrl.dragPieceColor_ = clickedPiece->getColor();
      ctrl.setState(std::make_unique<DraggingInteractionState>(
          clickedSquare, clickedPiece->getType(), clickedPiece->getColor()));
      return false;
    }
  }

  // Click on empty / opponent square — just clear selection
  ctrl.clearSelection();
  return false;
}
