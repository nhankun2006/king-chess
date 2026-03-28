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
  ChessColor color_;
  PieceType type_;
  bool moved_ = false;

public:
  Piece(ChessColor color, PieceType type) : color_(color), type_(type) {}
  virtual ~Piece() = default;

  ChessColor getColor() const { return color_; }
  PieceType getType() const { return type_; }
  bool hasMoved() const { return moved_; }
  void setMoved() { moved_ = true; }

  // Generate pseudo-legal moves (does NOT filter for self-check)
  virtual std::vector<Move> getPossibleMoves(const Board &board,
                                             Position pos) const = 0;

  // FEN-style symbol: uppercase = White, lowercase = Black
  virtual char getSymbol() const = 0;

  // Factory helper — create a piece by type
  static std::unique_ptr<Piece> create(PieceType type, ChessColor color);
};

// ─── Concrete Pieces ────────────────────────────────────────────────────────

class Pawn : public Piece {
public:
  explicit Pawn(ChessColor color) : Piece(color, PieceType::Pawn) {}
  std::vector<Move> getPossibleMoves(const Board &board,
                                     Position pos) const override;
  char getSymbol() const override {
    return (color_ == ChessColor::White) ? 'P' : 'p';
  }
};

class Knight : public Piece {
public:
  explicit Knight(ChessColor color) : Piece(color, PieceType::Knight) {}
  std::vector<Move> getPossibleMoves(const Board &board,
                                     Position pos) const override;
  char getSymbol() const override {
    return (color_ == ChessColor::White) ? 'N' : 'n';
  }
};

class Bishop : public Piece {
public:
  explicit Bishop(ChessColor color) : Piece(color, PieceType::Bishop) {}
  std::vector<Move> getPossibleMoves(const Board &board,
                                     Position pos) const override;
  char getSymbol() const override {
    return (color_ == ChessColor::White) ? 'B' : 'b';
  }
};

class Rook : public Piece {
public:
  explicit Rook(ChessColor color) : Piece(color, PieceType::Rook) {}
  std::vector<Move> getPossibleMoves(const Board &board,
                                     Position pos) const override;
  char getSymbol() const override {
    return (color_ == ChessColor::White) ? 'R' : 'r';
  }
};

class Queen : public Piece {
public:
  explicit Queen(ChessColor color) : Piece(color, PieceType::Queen) {}
  std::vector<Move> getPossibleMoves(const Board &board,
                                     Position pos) const override;
  char getSymbol() const override {
    return (color_ == ChessColor::White) ? 'Q' : 'q';
  }
};

class King : public Piece {
public:
  explicit King(ChessColor color) : Piece(color, PieceType::King) {}
  std::vector<Move> getPossibleMoves(const Board &board,
                                     Position pos) const override;
  char getSymbol() const override {
    return (color_ == ChessColor::White) ? 'K' : 'k';
  }
};

#endif // PIECE_H
