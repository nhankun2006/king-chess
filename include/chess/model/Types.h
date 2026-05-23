#ifndef TYPES_H
#define TYPES_H

enum class ChessColor { White, Black };

enum class PieceType { None, Pawn, Knight, Bishop, Rook, Queen, King };

enum class GameState { Playing, Check, Checkmate, Stalemate, Draw, Timeout };

struct Position {
  int row; // 0-7 (rank 1-8, white at bottom: row 0 = rank 1)
  int col; // 0-7 (file a-h)

  bool isValid() const { return row >= 0 && row < 8 && col >= 0 && col < 8; }

  bool operator==(const Position &other) const {
    return row == other.row && col == other.col;
  }

  bool operator!=(const Position &other) const { return !(*this == other); }
};

// Helper to flip color
inline ChessColor oppositeColor(ChessColor c) {
  return (c == ChessColor::White) ? ChessColor::Black : ChessColor::White;
}

// Helper to get string representation of PieceType
inline const char* getPieceName(PieceType type) {
  switch (type) {
    case PieceType::Pawn: return "Pawn";
    case PieceType::Knight: return "Knight";
    case PieceType::Bishop: return "Bishop";
    case PieceType::Rook: return "Rook";
    case PieceType::Queen: return "Queen";
    case PieceType::King: return "King";
    default: return "None";
  }
}

#endif // TYPES_H
