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
        view_->drawBoard(game_->getBoard(), selectedSquare_, selectedLegalMoves_,
                         restartConfirmOpen_, windowSizeDialogOpen_);
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
            default:
              break;
            }
            windowSizeDialogOpen_ = false;
          }
        }

        view_->drawBoard(game_->getBoard(), selectedSquare_, selectedLegalMoves_,
                         restartConfirmOpen_, windowSizeDialogOpen_);
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
          Move attemptedMove{*selectedSquare_, clickedSquare};
          const Piece *selectedPiece = game_->getBoard().getPieceAt(*selectedSquare_);

          if (selectedPiece && selectedPiece->getType() == PieceType::Pawn) {
            if ((selectedPiece->getColor() == Color::White &&
                 clickedSquare.row == 7) ||
                (selectedPiece->getColor() == Color::Black &&
                 clickedSquare.row == 0)) {
              attemptedMove.promotion = PieceType::Queen;
            }
          }

          if (game_->makeMove(attemptedMove)) {
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

    view_->drawBoard(game_->getBoard(), selectedSquare_, selectedLegalMoves_,
                     restartConfirmOpen_, windowSizeDialogOpen_);
  }
}
