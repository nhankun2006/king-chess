#ifndef BOARD_H
#define BOARD_H

#include "Types.h"
#include "Move.h"
#include "Piece.h"
#include <memory>
#include <string>
#include <vector>

class Renderer;

class Board {
private:
  Renderer *renderer_;
  std::unique_ptr<Piece> grid_[8][8];
  Position enPassantTarget_ = {-1, -1}; // Invalid = no en passant available

public:
  Board(Renderer *renderer);
  Board(const Board &other);            // Deep copy
  Board &operator=(const Board &other); // Deep copy assignment

  // Setup
  void setupInitialPosition();
  void clear();

  // Access
  Piece *getPieceAt(Position pos);
  const Piece *getPieceAt(Position pos) const;

  // Mutation
  void placePiece(Position pos, std::unique_ptr<Piece> piece);
  std::unique_ptr<Piece> removePiece(Position pos);
  void executeMove(const Move &move);

  // En passant target management
  Position getEnPassantTarget() const { return enPassantTarget_; }
  void setEnPassantTarget(Position pos) { enPassantTarget_ = pos; }

  // Queries
  bool isSquareAttacked(Position pos, Color byColor) const;
  Position findKing(Color color) const;
  bool isInCheck(Color color) const;

  // Serialization
  std::string toFEN() const; // Board portion of FEN (piece placement only)
};

#endif // BOARD_H
