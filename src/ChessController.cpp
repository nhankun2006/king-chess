#include "ChessControllder.h"

namespace {
constexpr int kBoardSizePx = 512;
constexpr int kCellSizePx = 64;

bool isInsideBoard(float x, float y) {
  return x >= 0.0f && x < static_cast<float>(kBoardSizePx) && y >= 0.0f &&
         y < static_cast<float>(kBoardSizePx);
}
} // namespace

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
      if (isInsideBoard(mousePos.x, mousePos.y)) {
        const Position clickedSquare = {
            static_cast<int>(mousePos.y) / kCellSizePx,
            static_cast<int>(mousePos.x) / kCellSizePx,
        };

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
