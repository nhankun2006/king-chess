#ifndef PIECE_H
#define PIECE_H

#include "Types.h"
#include "Move.h"
#include <memory>
#include <vector>

// Forward declaration
class Board;

// ─── Abstract Base ──────────────────────────────────────────────────────────

class Piece {
protected:
  Color color_;
  PieceType type_;
  bool moved_ = false;

public:
  Piece(Color color, PieceType type) : color_(color), type_(type) {}
  virtual ~Piece() = default;

  Color getColor() const { return color_; }
  PieceType getType() const { return type_; }
  bool hasMoved() const { return moved_; }
  void setMoved() { moved_ = true; }

  // Generate pseudo-legal moves (does NOT filter for self-check)
  virtual std::vector<Move> getPossibleMoves(const Board &board,
                                             Position pos) const = 0;

  // FEN-style symbol: uppercase = White, lowercase = Black
  virtual char getSymbol() const = 0;

  // Factory helper — create a piece by type
  static std::unique_ptr<Piece> create(PieceType type, Color color);
};

// ─── Concrete Pieces ────────────────────────────────────────────────────────

class Pawn : public Piece {
public:
  explicit Pawn(Color color) : Piece(color, PieceType::Pawn) {}
  std::vector<Move> getPossibleMoves(const Board &board,
                                     Position pos) const override;
  char getSymbol() const override {
    return (color_ == Color::White) ? 'P' : 'p';
  }
};

class Knight : public Piece {
public:
  explicit Knight(Color color) : Piece(color, PieceType::Knight) {}
  std::vector<Move> getPossibleMoves(const Board &board,
                                     Position pos) const override;
  char getSymbol() const override {
    return (color_ == Color::White) ? 'N' : 'n';
  }
};

class Bishop : public Piece {
public:
  explicit Bishop(Color color) : Piece(color, PieceType::Bishop) {}
  std::vector<Move> getPossibleMoves(const Board &board,
                                     Position pos) const override;
  char getSymbol() const override {
    return (color_ == Color::White) ? 'B' : 'b';
  }
};

class Rook : public Piece {
public:
  explicit Rook(Color color) : Piece(color, PieceType::Rook) {}
  std::vector<Move> getPossibleMoves(const Board &board,
                                     Position pos) const override;
  char getSymbol() const override {
    return (color_ == Color::White) ? 'R' : 'r';
  }
};

class Queen : public Piece {
public:
  explicit Queen(Color color) : Piece(color, PieceType::Queen) {}
  std::vector<Move> getPossibleMoves(const Board &board,
                                     Position pos) const override;
  char getSymbol() const override {
    return (color_ == Color::White) ? 'Q' : 'q';
  }
};

class King : public Piece {
public:
  explicit King(Color color) : Piece(color, PieceType::King) {}
  std::vector<Move> getPossibleMoves(const Board &board,
                                     Position pos) const override;
  char getSymbol() const override {
    return (color_ == Color::White) ? 'K' : 'k';
  }
};

#endif // PIECE_H
