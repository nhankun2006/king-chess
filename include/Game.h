#ifndef GAME_H
#define GAME_H

#include "Types.h"
#include "Move.h"
#include "Board.h"
#include "Observer.h"
#include <vector>

class Game {
private:
  Board board_;
  Color currentTurn_ = Color::White;
  GameState state_ = GameState::Playing;
  std::vector<Move> moveHistory_;
  std::vector<Observer *> observers_;

  // Castling rights: [White kingside, White queenside, Black kingside, Black
  // queenside]
  bool castlingRights_[4] = {true, true, true, true};

  // Internal helpers
  bool wouldBeInCheck(const Move &move, Color color) const;
  void updateCastlingRights(const Move &move);
  void updateGameState();
  void notifyMoveMade(Position from, Position to, bool isCapture);
  void notifyCheck(Color color);
  void notifyCheckmate(Color color);
  void notifyStalemate();
  void notifyDraw();

public:
  Game();

  // State queries
  GameState getState() const { return state_; }
  Color getCurrentTurn() const { return currentTurn_; }
  const Board &getBoard() const { return board_; }
  std::vector<Move> getMoveHistory() const { return moveHistory_; }

  // Castling rights access
  bool canCastleKingside(Color color) const;
  bool canCastleQueenside(Color color) const;

  // Move generation (fully legal — filters out self-check)
  std::vector<Move> getLegalMoves(Position pos) const;
  std::vector<Move> getAllLegalMoves(Color color) const;

  // Execute a move; returns false if the move is illegal
  bool makeMove(const Move &move);
  void restart();

  // Observer pattern
  void attach(Observer *observer);
  void detach(Observer *observer);
};

#endif // GAME_H
