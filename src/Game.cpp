#include "Game.h"

// ─── Constructor ────────────────────────────────────────────────────────────

Game::Game() { board_.setupInitialPosition(); }

// ─── Castling Rights Access ─────────────────────────────────────────────────

bool Game::canCastleKingside(Color color) const {
  return (color == Color::White) ? castlingRights_[0] : castlingRights_[2];
}

bool Game::canCastleQueenside(Color color) const {
  return (color == Color::White) ? castlingRights_[1] : castlingRights_[3];
}

// ─── Would a move leave the player in check? ────────────────────────────────

bool Game::wouldBeInCheck(const Move &move, Color color) const {
  // Simulate the move on a copy of the board
  Board testBoard(board_);
  testBoard.executeMove(move);
  return testBoard.isInCheck(color);
}

// ─── Legal Move Generation ──────────────────────────────────────────────────

std::vector<Move> Game::getLegalMoves(Position pos) const {
  std::vector<Move> legalMoves;

  const Piece *piece = board_.getPieceAt(pos);
  if (!piece || piece->getColor() != currentTurn_)
    return legalMoves;

  auto pseudoMoves = piece->getPossibleMoves(board_, pos);

  for (const auto &move : pseudoMoves) {
    // Filter castling: king must not be in check, and must not pass through
    // or land on an attacked square
    if (move.isCastling) {
      // Can't castle if rights are revoked
      if (move.to.col == 6 && !canCastleKingside(currentTurn_))
        continue;
      if (move.to.col == 2 && !canCastleQueenside(currentTurn_))
        continue;

      // Can't castle out of check
      if (board_.isInCheck(currentTurn_))
        continue;

      // Can't castle through check
      int rank = move.from.row;
      Color enemy = oppositeColor(currentTurn_);
      if (move.to.col == 6) {
        // Kingside: king passes through f-file
        if (board_.isSquareAttacked({rank, 5}, enemy))
          continue;
        if (board_.isSquareAttacked({rank, 6}, enemy))
          continue;
      } else {
        // Queenside: king passes through d-file
        if (board_.isSquareAttacked({rank, 3}, enemy))
          continue;
        if (board_.isSquareAttacked({rank, 2}, enemy))
          continue;
      }
    }

    // General filter: move must not leave own king in check
    if (!wouldBeInCheck(move, currentTurn_)) {
      legalMoves.push_back(move);
    }
  }

  return legalMoves;
}

std::vector<Move> Game::getAllLegalMoves(Color color) const {
  std::vector<Move> allMoves;

  // Temporary: we need to check moves for a specific color, which may differ
  // from currentTurn_ during state evaluation. We use a const cast-free
  // approach.
  for (int r = 0; r < 8; ++r) {
    for (int c = 0; c < 8; ++c) {
      const Piece *piece = board_.getPieceAt({r, c});
      if (!piece || piece->getColor() != color)
        continue;

      auto pseudoMoves = piece->getPossibleMoves(board_, {r, c});
      for (const auto &move : pseudoMoves) {
        // For castling, apply the same rights/check filters
        if (move.isCastling) {
          if (move.to.col == 6 && !canCastleKingside(color))
            continue;
          if (move.to.col == 2 && !canCastleQueenside(color))
            continue;
          if (board_.isInCheck(color))
            continue;

          int rank = move.from.row;
          Color enemy = oppositeColor(color);
          if (move.to.col == 6) {
            if (board_.isSquareAttacked({rank, 5}, enemy))
              continue;
            if (board_.isSquareAttacked({rank, 6}, enemy))
              continue;
          } else {
            if (board_.isSquareAttacked({rank, 3}, enemy))
              continue;
            if (board_.isSquareAttacked({rank, 2}, enemy))
              continue;
          }
        }

        // Must not leave own king in check
        Board testBoard(board_);
        testBoard.executeMove(move);
        if (!testBoard.isInCheck(color)) {
          allMoves.push_back(move);
        }
      }
    }
  }

  return allMoves;
}

// ─── Castling Rights Update ─────────────────────────────────────────────────

void Game::updateCastlingRights(const Move &move) {
  const Piece *movedPiece = board_.getPieceAt(move.to);
  if (!movedPiece)
    return;

  // King moved — lose both castling rights for that color
  if (movedPiece->getType() == PieceType::King) {
    if (movedPiece->getColor() == Color::White) {
      castlingRights_[0] = false; // White kingside
      castlingRights_[1] = false; // White queenside
    } else {
      castlingRights_[2] = false; // Black kingside
      castlingRights_[3] = false; // Black queenside
    }
  }

  // Rook moved or captured — lose that side's castling right
  auto revokeIfRookSquare = [&](Position pos) {
    if (pos == Position{0, 7})
      castlingRights_[0] = false; // White kingside
    if (pos == Position{0, 0})
      castlingRights_[1] = false; // White queenside
    if (pos == Position{7, 7})
      castlingRights_[2] = false; // Black kingside
    if (pos == Position{7, 0})
      castlingRights_[3] = false; // Black queenside
  };

  revokeIfRookSquare(move.from);
  revokeIfRookSquare(move.to); // captures a rook in its corner
}

// ─── Game State Detection ───────────────────────────────────────────────────

void Game::updateGameState() {
  Color nextPlayer = currentTurn_; // called after turn switch
  bool inCheck = board_.isInCheck(nextPlayer);
  auto legalMoves = getAllLegalMoves(nextPlayer);

  if (legalMoves.empty()) {
    state_ = inCheck ? GameState::Checkmate : GameState::Stalemate;
  } else {
    state_ = inCheck ? GameState::Check : GameState::Playing;
  }
}

// ─── Make Move ──────────────────────────────────────────────────────────────

bool Game::makeMove(const Move &move) {
  // 1) Verify the move is legal
  auto legalMoves = getLegalMoves(move.from);
  bool found = false;
  for (const auto &m : legalMoves) {
    if (m == move) {
      found = true;
      break;
    }
  }
  if (!found)
    return false;

  // 2) Execute the move on the board
  board_.executeMove(move);

  // 3) Update castling rights (must be done after move execution
  //    because we need to inspect the moved piece at its new position)
  updateCastlingRights(move);

  // 4) Record the move
  moveHistory_.push_back(move);

  // 5) Switch turn
  currentTurn_ = oppositeColor(currentTurn_);

  // 6) Detect check / checkmate / stalemate for the next player
  updateGameState();

  return true;
}

void Game::restart() {
  board_.clear();
  board_.setupInitialPosition();
  currentTurn_ = Color::White;
  state_ = GameState::Playing;
  moveHistory_.clear();
  castlingRights_[0] = true;
  castlingRights_[1] = true;
  castlingRights_[2] = true;
  castlingRights_[3] = true;
}
