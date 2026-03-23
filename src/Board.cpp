#include "Board.h"
#include <sstream>

// ─── Constructor ────────────────────────────────────────────────────────────

Board::Board() {
  for (int r = 0; r < 8; ++r)
    for (int c = 0; c < 8; ++c)
      grid_[r][c] = nullptr;
}

// ─── Deep Copy ──────────────────────────────────────────────────────────────

Board::Board(const Board &other) : enPassantTarget_(other.enPassantTarget_) {
  for (int r = 0; r < 8; ++r) {
    for (int c = 0; c < 8; ++c) {
      if (other.grid_[r][c]) {
        grid_[r][c] = Piece::create(other.grid_[r][c]->getType(),
                                    other.grid_[r][c]->getColor());
        if (other.grid_[r][c]->hasMoved())
          grid_[r][c]->setMoved();
      } else {
        grid_[r][c] = nullptr;
      }
    }
  }
}

Board &Board::operator=(const Board &other) {
  if (this == &other)
    return *this;
  enPassantTarget_ = other.enPassantTarget_;
  for (int r = 0; r < 8; ++r) {
    for (int c = 0; c < 8; ++c) {
      if (other.grid_[r][c]) {
        grid_[r][c] = Piece::create(other.grid_[r][c]->getType(),
                                    other.grid_[r][c]->getColor());
        if (other.grid_[r][c]->hasMoved())
          grid_[r][c]->setMoved();
      } else {
        grid_[r][c] = nullptr;
      }
    }
  }
  return *this;
}

// ─── Setup ──────────────────────────────────────────────────────────────────

void Board::clear() {
  for (int r = 0; r < 8; ++r)
    for (int c = 0; c < 8; ++c)
      grid_[r][c] = nullptr;
  enPassantTarget_ = {-1, -1};
}

void Board::setupInitialPosition() {
  clear();

  // Back ranks piece order: Rook, Knight, Bishop, Queen, King, Bishop, Knight,
  // Rook
  static const PieceType backRank[] = {
      PieceType::Rook, PieceType::Knight, PieceType::Bishop, PieceType::Queen,
      PieceType::King, PieceType::Bishop, PieceType::Knight, PieceType::Rook};

  for (int c = 0; c < 8; ++c) {
    // White back rank (row 0 = rank 1)
    grid_[0][c] = Piece::create(backRank[c], ChessColor::White);
    // White pawns (row 1 = rank 2)
    grid_[1][c] = Piece::create(PieceType::Pawn, ChessColor::White);
    // Black pawns (row 6 = rank 7)
    grid_[6][c] = Piece::create(PieceType::Pawn, ChessColor::Black);
    // Black back rank (row 7 = rank 8)
    grid_[7][c] = Piece::create(backRank[c], ChessColor::Black);
  }
}

// ─── Access ─────────────────────────────────────────────────────────────────

Piece *Board::getPieceAt(Position pos) {
  if (!pos.isValid())
    return nullptr;
  return grid_[pos.row][pos.col].get();
}

const Piece *Board::getPieceAt(Position pos) const {
  if (!pos.isValid())
    return nullptr;
  return grid_[pos.row][pos.col].get();
}

// ─── Mutation ───────────────────────────────────────────────────────────────

void Board::placePiece(Position pos, std::unique_ptr<Piece> piece) {
  if (pos.isValid()) {
    grid_[pos.row][pos.col] = std::move(piece);
  }
}

std::unique_ptr<Piece> Board::removePiece(Position pos) {
  if (!pos.isValid())
    return nullptr;
  return std::move(grid_[pos.row][pos.col]);
}

void Board::executeMove(const Move &move) {
  // 1) Handle en passant capture — remove the captured pawn
  if (move.isEnPassant) {
    int capturedRow =
        move.from.row; // the pawn that is captured is on the same rank
    grid_[capturedRow][move.to.col] = nullptr;
  }

  // 2) Handle castling — move the rook
  if (move.isCastling) {
    int rank = move.from.row;
    if (move.to.col == 6) {
      // Kingside: rook h -> f
      grid_[rank][5] = std::move(grid_[rank][7]);
      grid_[rank][5]->setMoved();
    } else if (move.to.col == 2) {
      // Queenside: rook a -> d
      grid_[rank][3] = std::move(grid_[rank][0]);
      grid_[rank][3]->setMoved();
    }
  }

  // 3) Move the piece
  grid_[move.to.row][move.to.col] =
      std::move(grid_[move.from.row][move.from.col]);
  grid_[move.to.row][move.to.col]->setMoved();

  // 4) Handle promotion
  if (move.promotion != PieceType::None) {
    ChessColor color = grid_[move.to.row][move.to.col]->getColor();
    grid_[move.to.row][move.to.col] = Piece::create(move.promotion, color);
    grid_[move.to.row][move.to.col]->setMoved();
  }

  // 5) Update en passant target
  const Piece *movedPiece = grid_[move.to.row][move.to.col].get();
  if (movedPiece->getType() == PieceType::Pawn &&
      std::abs(move.to.row - move.from.row) == 2) {
    // Pawn double push — set en passant target to the square behind
    int epRow = (move.from.row + move.to.row) / 2;
    enPassantTarget_ = {epRow, move.from.col};
  } else {
    enPassantTarget_ = {-1, -1};
  }
}

// ─── Queries ────────────────────────────────────────────────────────────────

bool Board::isSquareAttacked(Position pos, ChessColor byColor) const {
  for (int r = 0; r < 8; ++r) {
    for (int c = 0; c < 8; ++c) {
      const Piece *piece = grid_[r][c].get();
      if (!piece || piece->getColor() != byColor)
        continue;

      auto moves = piece->getPossibleMoves(*this, {r, c});
      for (const auto &m : moves) {
        // Skip castling moves — they don't "attack" squares
        if (m.isCastling)
          continue;
        if (m.to == pos)
          return true;
      }
    }
  }
  return false;
}

Position Board::findKing(ChessColor color) const {
  for (int r = 0; r < 8; ++r) {
    for (int c = 0; c < 8; ++c) {
      const Piece *piece = grid_[r][c].get();
      if (piece && piece->getType() == PieceType::King &&
          piece->getColor() == color) {
        return {r, c};
      }
    }
  }
  return {-1, -1}; // Should never happen in a valid game
}

bool Board::isInCheck(ChessColor color) const {
  Position kingPos = findKing(color);
  return isSquareAttacked(kingPos, oppositeColor(color));
}

// ─── FEN (piece placement only) ─────────────────────────────────────────────

std::string Board::toFEN() const {
  std::ostringstream fen;

  for (int r = 7; r >= 0; --r) { // FEN starts from rank 8 (row 7)
    int emptyCount = 0;
    for (int c = 0; c < 8; ++c) {
      const Piece *piece = grid_[r][c].get();
      if (!piece) {
        ++emptyCount;
      } else {
        if (emptyCount > 0) {
          fen << emptyCount;
          emptyCount = 0;
        }
        fen << piece->getSymbol();
      }
    }
    if (emptyCount > 0)
      fen << emptyCount;
    if (r > 0)
      fen << '/';
  }

  return fen.str();
}
