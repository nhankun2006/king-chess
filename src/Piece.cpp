#include "Piece.h"
#include "Board.h"

// ─── Factory ────────────────────────────────────────────────────────────────

std::unique_ptr<Piece> Piece::create(PieceType type, ChessColor color) {
  switch (type) {
  case PieceType::Pawn:
    return std::make_unique<Pawn>(color);
  case PieceType::Knight:
    return std::make_unique<Knight>(color);
  case PieceType::Bishop:
    return std::make_unique<Bishop>(color);
  case PieceType::Rook:
    return std::make_unique<Rook>(color);
  case PieceType::Queen:
    return std::make_unique<Queen>(color);
  case PieceType::King:
    return std::make_unique<King>(color);
  default:
    return nullptr;
  }
}

// ─── Helper: add move if target square is empty or has an enemy ─────────────

static bool addMoveIfValid(std::vector<Move> &moves, const Board &board,
                           Position from, Position to, ChessColor ownColor) {
  if (!to.isValid())
    return false;

  const Piece *target = board.getPieceAt(to);
  if (target && target->getColor() == ownColor)
    return false; // blocked by own piece

  moves.push_back({from, to});
  return (target ==
          nullptr); // true = square was empty (sliding pieces keep going)
}

// ─── Helper: slide in a direction until blocked ─────────────────────────────

static void addSlidingMoves(std::vector<Move> &moves, const Board &board,
                            Position from, ChessColor ownColor, int dRow,
                            int dCol) {
  Position to = {from.row + dRow, from.col + dCol};
  while (to.isValid()) {
    const Piece *target = board.getPieceAt(to);
    if (target) {
      if (target->getColor() != ownColor) {
        moves.push_back({from, to}); // capture
      }
      break; // blocked either way
    }
    moves.push_back({from, to});
    to.row += dRow;
    to.col += dCol;
  }
}

// ─── Pawn ───────────────────────────────────────────────────────────────────

static void addPawnMoves(std::vector<Move> &moves, Position from, Position to,
                         ChessColor color) {
  // Check if this is a promotion rank
  int promoRank = (color == ChessColor::White) ? 7 : 0;
  if (to.row == promoRank) {
    // Generate one move for each promotion choice
    for (PieceType pt : {PieceType::Queen, PieceType::Rook, PieceType::Bishop,
                         PieceType::Knight}) {
      moves.push_back({from, to, pt, false, false});
    }
  } else {
    moves.push_back({from, to});
  }
}

std::vector<Move> Pawn::getPossibleMoves(const Board &board,
                                         Position pos) const {
  std::vector<Move> moves;
  int direction = (color_ == ChessColor::White) ? 1 : -1;
  int startRank = (color_ == ChessColor::White) ? 1 : 6;

  // 1) Single push forward
  Position oneStep = {pos.row + direction, pos.col};
  if (oneStep.isValid() && !board.getPieceAt(oneStep)) {
    addPawnMoves(moves, pos, oneStep, color_);

    // 2) Double push from starting rank
    Position twoStep = {pos.row + 2 * direction, pos.col};
    if (pos.row == startRank && !board.getPieceAt(twoStep)) {
      moves.push_back({pos, twoStep});
    }
  }

  // 3) Diagonal captures
  for (int dc : {-1, 1}) {
    Position cap = {pos.row + direction, pos.col + dc};
    if (!cap.isValid())
      continue;

    const Piece *target = board.getPieceAt(cap);
    if (target && target->getColor() != color_) {
      addPawnMoves(moves, pos, cap, color_);
    }

    // 4) En passant
    if (cap == board.getEnPassantTarget()) {
      moves.push_back({pos, cap, PieceType::None, false, true});
    }
  }

  return moves;
}

// ─── Knight ─────────────────────────────────────────────────────────────────

std::vector<Move> Knight::getPossibleMoves(const Board &board,
                                           Position pos) const {
  std::vector<Move> moves;
  static const int offsets[][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                                   {1, -2},  {1, 2},  {2, -1},  {2, 1}};

  for (auto &off : offsets) {
    Position to = {pos.row + off[0], pos.col + off[1]};
    addMoveIfValid(moves, board, pos, to, color_);
  }
  return moves;
}

// ─── Bishop ─────────────────────────────────────────────────────────────────

std::vector<Move> Bishop::getPossibleMoves(const Board &board,
                                           Position pos) const {
  std::vector<Move> moves;
  for (auto [dr, dc] : std::initializer_list<std::pair<int, int>>{
           {1, 1}, {1, -1}, {-1, 1}, {-1, -1}}) {
    addSlidingMoves(moves, board, pos, color_, dr, dc);
  }
  return moves;
}

// ─── Rook ───────────────────────────────────────────────────────────────────

std::vector<Move> Rook::getPossibleMoves(const Board &board,
                                         Position pos) const {
  std::vector<Move> moves;
  for (auto [dr, dc] : std::initializer_list<std::pair<int, int>>{
           {1, 0}, {-1, 0}, {0, 1}, {0, -1}}) {
    addSlidingMoves(moves, board, pos, color_, dr, dc);
  }
  return moves;
}

// ─── Queen ──────────────────────────────────────────────────────────────────

std::vector<Move> Queen::getPossibleMoves(const Board &board,
                                          Position pos) const {
  std::vector<Move> moves;
  for (auto [dr, dc] : std::initializer_list<std::pair<int, int>>{{1, 0},
                                                                  {-1, 0},
                                                                  {0, 1},
                                                                  {0, -1},
                                                                  {1, 1},
                                                                  {1, -1},
                                                                  {-1, 1},
                                                                  {-1, -1}}) {
    addSlidingMoves(moves, board, pos, color_, dr, dc);
  }
  return moves;
}

// ─── King ───────────────────────────────────────────────────────────────────

std::vector<Move> King::getPossibleMoves(const Board &board,
                                         Position pos) const {
  std::vector<Move> moves;

  // 1) Normal one-square moves in all 8 directions
  for (int dr = -1; dr <= 1; ++dr) {
    for (int dc = -1; dc <= 1; ++dc) {
      if (dr == 0 && dc == 0)
        continue;
      Position to = {pos.row + dr, pos.col + dc};
      addMoveIfValid(moves, board, pos, to, color_);
    }
  }

  // 2) Castling — pseudo-legal (Game class will verify legality)
  //    King must not have moved, and path must be clear.
  //    We do NOT check for check here — that's the Game's job.
  if (!moved_) {
    int rank = (color_ == ChessColor::White) ? 0 : 7;

    // Kingside: king at e -> g, rook at h -> f
    const Piece *kRook = board.getPieceAt({rank, 7});
    if (kRook && kRook->getType() == PieceType::Rook && !kRook->hasMoved()) {
      if (!board.getPieceAt({rank, 5}) && !board.getPieceAt({rank, 6})) {
        moves.push_back({pos, {rank, 6}, PieceType::None, true, false});
      }
    }

    // Queenside: king at e -> c, rook at a -> d
    const Piece *qRook = board.getPieceAt({rank, 0});
    if (qRook && qRook->getType() == PieceType::Rook && !qRook->hasMoved()) {
      if (!board.getPieceAt({rank, 1}) && !board.getPieceAt({rank, 2}) &&
          !board.getPieceAt({rank, 3})) {
        moves.push_back({pos, {rank, 2}, PieceType::None, true, false});
      }
    }
  }

  return moves;
}
