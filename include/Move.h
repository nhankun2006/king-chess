#ifndef MOVE_H
#define MOVE_H

#include "Types.h"
#include <string>

struct Move {
  Position from;
  Position to;
  PieceType promotion = PieceType::None; // Non-None only for pawn promotion
  bool isCastling = false;
  bool isEnPassant = false;

  bool operator==(const Move &other) const {
    return from == other.from && to == other.to &&
           promotion == other.promotion && isCastling == other.isCastling &&
           isEnPassant == other.isEnPassant;
  }

  bool operator!=(const Move &other) const { return !(*this == other); }

  // Convert to long algebraic notation, e.g. "e2e4", "e7e8q"
  std::string toAlgebraic() const {
    std::string result;
    result += static_cast<char>('a' + from.col);
    result += static_cast<char>('1' + from.row);
    result += static_cast<char>('a' + to.col);
    result += static_cast<char>('1' + to.row);

    if (promotion != PieceType::None) {
      switch (promotion) {
      case PieceType::Queen:
        result += 'q';
        break;
      case PieceType::Rook:
        result += 'r';
        break;
      case PieceType::Bishop:
        result += 'b';
        break;
      case PieceType::Knight:
        result += 'n';
        break;
      default:
        break;
      }
    }
    return result;
  }
};

#endif // MOVE_H
