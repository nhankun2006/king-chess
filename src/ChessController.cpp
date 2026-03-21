#include "ChessControllder.h"

void ChessController::updateSelection(Position pos) {
  selectedSquare_ = pos;
  selectedLegalMoves_ = game_->getLegalMoves(pos);
}

void ChessController::clearSelection() {
  selectedSquare_.reset();
  selectedLegalMoves_.clear();
}

void ChessController::run() {
  auto getActiveCastlingTween = [&]() -> std::optional<ChessView::CastlingTween> {
    if (!castlingTween_.has_value()) {
      return std::nullopt;
    }

    ChessView::CastlingTween tween = *castlingTween_;
    const double elapsed = GetTime() - castlingTweenStartTime_;
    tween.progress = static_cast<float>(elapsed / castlingTweenDurationSeconds_);
    if (tween.progress >= 1.0f) {
      castlingTween_.reset();
      return std::nullopt;
    }
    if (tween.progress < 0.0f) {
      tween.progress = 0.0f;
    }
    return tween;
  };

  auto drawFrame = [&]() {
    view_->drawBoard(game_->getBoard(), selectedSquare_, selectedLegalMoves_,
                     restartConfirmOpen_, windowSizeDialogOpen_,
                     getActiveCastlingTween());
  };

  while (!WindowShouldClose()) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      const Vector2 mousePos = GetMousePosition();

      if (restartConfirmOpen_) {
        if (view_->isRestartConfirmYesClicked(mousePos.x, mousePos.y)) {
          game_->restart();
          clearSelection();
          restartConfirmOpen_ = false;
        } else if (view_->isRestartConfirmNoClicked(mousePos.x, mousePos.y)) {
          restartConfirmOpen_ = false;
        }
        drawFrame();
        continue;
      }

      if (windowSizeDialogOpen_) {
        if (view_->isWindowSizeDialogCloseClicked(mousePos.x, mousePos.y)) {
          windowSizeDialogOpen_ = false;
        } else {
          const auto selectedSizeOpt =
              view_->getWindowSizeOptionClicked(mousePos.x, mousePos.y);
          if (selectedSizeOpt.has_value()) {
            switch (*selectedSizeOpt) {
            case 0:
              SetWindowSize(700, 512);
              break;
            case 1:
              SetWindowSize(900, 650);
              break;
            case 2:
              SetWindowSize(1100, 780);
              break;
            case 3:
              SetWindowSize(1300, 920);
              break;
            default:
              break;
            }
            windowSizeDialogOpen_ = false;
          }
        }

        drawFrame();
        continue;
      }

      bool handledUiClick = false;

      if (view_->isSettingsButtonClicked(mousePos.x, mousePos.y)) {
        windowSizeDialogOpen_ = true;
        handledUiClick = true;
      }

      if (!handledUiClick &&
          view_->isRotateButtonClicked(mousePos.x, mousePos.y)) {
        view_->toggleBoardOrientation();
        handledUiClick = true;
      }

      if (!handledUiClick &&
          view_->isRestartButtonClicked(mousePos.x, mousePos.y)) {
        windowSizeDialogOpen_ = false;
        restartConfirmOpen_ = true;
        handledUiClick = true;
      }

      if (!handledUiClick) {
        const auto clickedSquareOpt =
            view_->screenToBoardSquare(mousePos.x, mousePos.y);
        if (clickedSquareOpt.has_value()) {
        const Position clickedSquare = *clickedSquareOpt;

        const Piece *clickedPiece = game_->getBoard().getPieceAt(clickedSquare);

        if (!selectedSquare_.has_value()) {
          if (clickedPiece && clickedPiece->getColor() == game_->getCurrentTurn()) {
            updateSelection(clickedSquare);
          }
        } else {
          const Piece *selectedPiece = game_->getBoard().getPieceAt(*selectedSquare_);
          std::optional<Move> attemptedMove;

          const bool isPromotionSquare =
              selectedPiece && selectedPiece->getType() == PieceType::Pawn &&
              ((selectedPiece->getColor() == Color::White && clickedSquare.row == 7) ||
               (selectedPiece->getColor() == Color::Black && clickedSquare.row == 0));

          for (const auto &legalMove : selectedLegalMoves_) {
            if (legalMove.to != clickedSquare) {
              continue;
            }

            if (isPromotionSquare) {
              if (legalMove.promotion == PieceType::Queen) {
                attemptedMove = legalMove;
                break;
              }
              if (!attemptedMove.has_value()) {
                attemptedMove = legalMove;
              }
            } else {
              if (legalMove.promotion == PieceType::None) {
                attemptedMove = legalMove;
                break;
              }
              if (!attemptedMove.has_value()) {
                attemptedMove = legalMove;
              }
            }
          }

          if (attemptedMove.has_value() && game_->makeMove(*attemptedMove)) {
            if (attemptedMove->isCastling && selectedPiece != nullptr &&
                selectedPiece->getType() == PieceType::King) {
              ChessView::CastlingTween tween;
              tween.color = selectedPiece->getColor();
              tween.kingFrom = attemptedMove->from;
              tween.kingTo = attemptedMove->to;
              const int rookFromCol = (attemptedMove->to.col == 6) ? 7 : 0;
              const int rookToCol = (attemptedMove->to.col == 6) ? 5 : 3;
              tween.rookFrom = {attemptedMove->from.row, rookFromCol};
              tween.rookTo = {attemptedMove->from.row, rookToCol};
              tween.progress = 0.0f;
              castlingTween_ = tween;
              castlingTweenStartTime_ = GetTime();
            }
            clearSelection();
          } else if (clickedPiece &&
                     clickedPiece->getColor() == game_->getCurrentTurn()) {
            updateSelection(clickedSquare);
          } else {
            clearSelection();
          }
        }
      }
      }
    }

    drawFrame();
  }
}
