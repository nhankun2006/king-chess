#include "ChessController.h"

#include <cstdio>

#include "UIConfig.h"

void ChessController::updateSelection(Position pos) {
  selectedSquare_ = pos;
  selectedLegalMoves_ = game_->getLegalMoves(pos);
}

void ChessController::clearSelection() {
  selectedSquare_.reset();
  selectedLegalMoves_.clear();
}

void ChessController::stopDragging() {
  isDraggingPiece_ = false;
  dragFromSquare_.reset();
  dragPieceType_ = PieceType::None;
}

void ChessController::triggerInvalidMoveWarning(
    const std::optional<Position> &fallbackSquare) {
  Position warningSquare = {-1, -1};
  const ChessColor sideToMove = game_->getCurrentTurn();
  const Board &board = game_->getBoard();

  if (board.isInCheck(sideToMove)) {
    warningSquare = board.findKing(sideToMove);
  } else if (fallbackSquare.has_value()) {
    warningSquare = fallbackSquare.value();
  } else if (selectedSquare_.has_value()) {
    warningSquare = selectedSquare_.value();
  }

  if (warningSquare.row >= 0 && warningSquare.row < 8 && warningSquare.col >= 0 &&
      warningSquare.col < 8) {
    view_->triggerInvalidHighlight(warningSquare);
  }
}

bool ChessController::processInput() {
  bool movedThisFrame = false;

  if (IsKeyPressed(KEY_S)) {
    if (game_->saveGame("save.bin")) {
      view_->triggerSaveMessage();
    }
  }

  if (autosavePeriodic_) {
    const double now = GetTime();
    if (now - lastAutosaveTime_ >= autosaveIntervalSeconds_) {
      if (game_->saveGame("save.bin")) {
        view_->triggerSaveMessage();
      }
      lastAutosaveTime_ = now;
    }
  }

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    const Vector2 mousePos = GetMousePosition();

    if (promotionPromptOpen_) {
      const PieceType selectedPromotion =
          view_->getPromotionOptionClicked(mousePos.x, mousePos.y);
      if (selectedPromotion != PieceType::None) {
        for (const auto &move : pendingPromotionMoves_) {
          if (move.promotion != selectedPromotion) {
            continue;
          }

          const bool willCapture =
              move.isEnPassant || (game_->getBoard().getPieceAt(move.to) != nullptr);
          if (!game_->makeMove(move)) {
            continue;
          }

          view_->onMoveApplied(move, willCapture);
          promotionPromptOpen_ = false;
          pendingPromotionMoves_.clear();
          clearSelection();
          stopDragging();
          movedThisFrame = true;
          break;
        }
      } else {
        triggerInvalidMoveWarning(std::nullopt);
      }

      return false;
    }

    if (restartConfirmOpen_) {
      if (view_->isRestartConfirmYesClicked(mousePos.x, mousePos.y)) {
        game_->restart();
        view_->resetVisualEffects();
        clearSelection();
        stopDragging();
        restartConfirmOpen_ = false;
      } else if (view_->isRestartConfirmNoClicked(mousePos.x, mousePos.y)) {
        restartConfirmOpen_ = false;
      }
      return false;
    }

    if (windowSizeDialogOpen_) {
      if (view_->isWindowSizeDialogCloseClicked(mousePos.x, mousePos.y)) {
        windowSizeDialogOpen_ = false;
      } else if (view_->isExitToMenuButtonClicked(mousePos.x, mousePos.y)) {
        return true;
      } else {
        const int selectedSizeOpt =
            view_->getWindowSizeOptionClicked(mousePos.x, mousePos.y);
        if (selectedSizeOpt >= 0 && selectedSizeOpt < ui::Window::kSizePresetCount) {
          const ui::WindowPreset preset = ui::Window::kSizePresets[selectedSizeOpt];
          SetWindowSize(preset.width, preset.height);

          FILE *f = std::fopen("settings.ini", "w");
          if (f != nullptr) {
            std::fprintf(f, "%d %d\n", preset.width, preset.height);
            std::fclose(f);
          }
          windowSizeDialogOpen_ = false;
        }
      }
      return false;
    }

    bool handledUiClick = false;

    if (view_->isSettingsButtonClicked(mousePos.x, mousePos.y)) {
      stopDragging();
      windowSizeDialogOpen_ = true;
      handledUiClick = true;
    }

    if (!handledUiClick && view_->isRotateButtonClicked(mousePos.x, mousePos.y)) {
      stopDragging();
      view_->toggleBoardOrientation();
      handledUiClick = true;
    }

    if (!handledUiClick && view_->isRestartButtonClicked(mousePos.x, mousePos.y)) {
      stopDragging();
      windowSizeDialogOpen_ = false;
      restartConfirmOpen_ = true;
      handledUiClick = true;
    }

    if (!handledUiClick && view_->isUndoButtonClicked(mousePos.x, mousePos.y)) {
      stopDragging();
      windowSizeDialogOpen_ = false;
      if (game_->undo()) {
        view_->clearCaptureEffects();
      }
      handledUiClick = true;
    }

    if (!handledUiClick) {
      Position clickedSquare;
      if (view_->screenToBoardSquare(mousePos.x, mousePos.y, clickedSquare)) {
        const Piece *clickedPiece = game_->getBoard().getPieceAt(clickedSquare);
        if (clickedPiece != nullptr &&
            clickedPiece->getColor() == game_->getCurrentTurn()) {
          updateSelection(clickedSquare);
          isDraggingPiece_ = true;
          dragFromSquare_ = clickedSquare;
          dragPieceType_ = clickedPiece->getType();
          dragPieceColor_ = clickedPiece->getColor();
        } else {
          clearSelection();
          stopDragging();
        }
      } else {
        clearSelection();
        stopDragging();
      }
    }
  }

  if (isDraggingPiece_ && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    const Vector2 mousePos = GetMousePosition();
    Position dropSquare{};
    const bool hasDropSquare =
        view_->screenToBoardSquare(mousePos.x, mousePos.y, dropSquare);

    Move attemptedMove{};
    bool hasAttemptedMove = false;
    const Piece *selectedPiece =
        selectedSquare_.has_value() ? game_->getBoard().getPieceAt(selectedSquare_.value())
                                    : nullptr;

    if (hasDropSquare && selectedPiece != nullptr) {
      std::vector<Move> candidateMoves;
      for (const auto &legalMove : selectedLegalMoves_) {
        if (legalMove.to == dropSquare) {
          candidateMoves.push_back(legalMove);
        }
      }

      if (!candidateMoves.empty()) {
        if (candidateMoves.size() == 1 || selectedPiece->getType() != PieceType::Pawn) {
          attemptedMove = candidateMoves.front();
          hasAttemptedMove = true;
        } else {
          bool hasPromotionChoices = false;
          for (const auto &move : candidateMoves) {
            if (move.promotion != PieceType::None) {
              hasPromotionChoices = true;
              break;
            }
          }

          if (hasPromotionChoices) {
            promotionPromptOpen_ = true;
            promotionPromptColor_ = selectedPiece->getColor();
            pendingPromotionMoves_.clear();
            for (const auto &move : candidateMoves) {
              if (move.promotion != PieceType::None) {
                pendingPromotionMoves_.push_back(move);
              }
            }
          } else {
            attemptedMove = candidateMoves.front();
            hasAttemptedMove = true;
          }
        }
      }
    }

    if (!promotionPromptOpen_ && hasAttemptedMove) {
      const bool willCapture =
          attemptedMove.isEnPassant ||
          (game_->getBoard().getPieceAt(attemptedMove.to) != nullptr);
      if (game_->makeMove(attemptedMove)) {
        view_->onMoveApplied(attemptedMove, willCapture);
        movedThisFrame = true;

        if (attemptedMove.isCastling && selectedPiece != nullptr &&
            selectedPiece->getType() == PieceType::King) {
          const int rookFromCol = (attemptedMove.to.col == 6) ? 7 : 0;
          const int rookToCol = (attemptedMove.to.col == 6) ? 5 : 3;
          view_->triggerCastlingTween(selectedPiece->getColor(), attemptedMove.from,
                                      attemptedMove.to,
                                      {attemptedMove.from.row, rookFromCol},
                                      {attemptedMove.from.row, rookToCol});
        }
      }
    } else if (!promotionPromptOpen_) {
      if (hasDropSquare && selectedSquare_.has_value() &&
          !(dropSquare == selectedSquare_.value())) {
        triggerInvalidMoveWarning(dropSquare);
      }
    }

    if (!promotionPromptOpen_) {
      clearSelection();
    }
    stopDragging();
  }

  if (movedThisFrame && autosaveOnMove_) {
    if (game_->saveGame("save.bin")) {
      view_->triggerSaveMessage();
    }
  }

  return false;
}

void ChessController::render() {
  DragPreview dragPreviewVal{};
  DragPreview *dragPreview = nullptr;
  if (isDraggingPiece_ && dragFromSquare_.has_value() &&
      dragPieceType_ != PieceType::None) {
    dragPreviewVal.type = dragPieceType_;
    dragPreviewVal.color = dragPieceColor_;
    dragPreviewVal.from = dragFromSquare_.value();
    dragPreviewVal.mousePos = GetMousePosition();
    dragPreview = &dragPreviewVal;
  }

  ChessColor winnerColorVal = ChessColor::White;
  ChessColor *winnerColor = nullptr;
  const GameState gameState = game_->getState();
  if (gameState == GameState::Checkmate) {
    winnerColorVal = oppositeColor(game_->getCurrentTurn());
    winnerColor = &winnerColorVal;
  }

  ChessColor *promotionColor = nullptr;
  if (promotionPromptOpen_) {
    promotionColor = &promotionPromptColor_;
  }

  view_->drawBoard(game_->getBoard(),
                   selectedSquare_.has_value() ? &selectedSquare_.value() : nullptr,
                   selectedLegalMoves_, restartConfirmOpen_, windowSizeDialogOpen_,
                   gameState, winnerColor, dragPreview, promotionColor,
                   true);
}
