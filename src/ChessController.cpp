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

    view_->drawBoard(game_->getBoard(), selectedSquare_, selectedLegalMoves_);
  }
}
