#ifndef CHESSCONTROLLER_H
#define CHESSCONTROLLER_H

#include <optional>
#include <vector>

#include "Game.h"
#include "ChessView.h"

class ChessController {
private:
  Game *game_ = nullptr;
  ChessView *view_ = nullptr;
  std::optional<Position> selectedSquare_;
  std::vector<Move> selectedLegalMoves_;
  bool restartConfirmOpen_ = false;

  void updateSelection(Position pos);
  void clearSelection();

public:
  ChessController(Game &game, ChessView &view) : game_(&game), view_(&view) {}

  void run();
};

#endif // CHESSCONTROLLER_H
