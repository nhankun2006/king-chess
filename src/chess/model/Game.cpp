#include "chess/model/Game.h"

#include <algorithm>
#include <fstream>
#include <optional>

// ─── Constructor ────────────────────────────────────────────────────────────

Game::Game() { board_.setupInitialPosition(); }

// ─── Castling Rights Access ─────────────────────────────────────────────────

bool Game::canCastleKingside(ChessColor color) const {
  return (color == ChessColor::White) ? castlingRights_[0] : castlingRights_[2];
}

bool Game::canCastleQueenside(ChessColor color) const {
  return (color == ChessColor::White) ? castlingRights_[1] : castlingRights_[3];
}

// ─── Would a move leave the player in check? ────────────────────────────────

bool Game::wouldBeInCheck(const Move &move, ChessColor color) const {
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
      ChessColor enemy = oppositeColor(currentTurn_);
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

std::vector<Move> Game::getLegalMovesBetween(Position from, Position to) const {
  std::vector<Move> filteredMoves;
  auto legalMoves = getLegalMoves(from);

  for (const auto &move : legalMoves) {
    if (move.to == to) {
      filteredMoves.push_back(move);
    }
  }

  return filteredMoves;
}

bool Game::hasPromotionChoices(Position from, Position to) const {
  auto candidates = getLegalMovesBetween(from, to);
  for (const auto &move : candidates) {
    if (move.promotion != PieceType::None) {
      return true;
    }
  }
  return false;
}

std::optional<Move> Game::resolveLegalMove(Position from, Position to,
                                           PieceType promotion) const {
  auto candidates = getLegalMovesBetween(from, to);
  if (candidates.empty()) {
    return std::nullopt;
  }

  if (promotion == PieceType::None) {
    for (const auto &move : candidates) {
      if (move.promotion == PieceType::None) {
        return move;
      }
    }

    if (candidates.size() == 1) {
      return candidates.front();
    }

    return std::nullopt;
  }

  for (const auto &move : candidates) {
    if (move.promotion == promotion) {
      return move;
    }
  }

  return std::nullopt;
}

std::vector<Move> Game::getAllLegalMoves(ChessColor color) const {
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
          ChessColor enemy = oppositeColor(color);
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
    if (movedPiece->getColor() == ChessColor::White) {
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
  ChessColor nextPlayer = currentTurn_; // called after turn switch
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
  if (state_ == GameState::Checkmate || state_ == GameState::Stalemate ||
      state_ == GameState::Draw) {
    return false;
  }

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

  // 1.5) Save snapshot for Undo
  undoStack_.push_back({
    board_, currentTurn_, state_,
    {castlingRights_[0], castlingRights_[1], castlingRights_[2], castlingRights_[3]},
    whiteTimeLeft_, blackTimeLeft_,
    capturedWhitePieces_, capturedBlackPieces_
  });

  // 2) Detect capture before executing (target square or en passant)
  bool isCapture = move.isEnPassant;
  if (!isCapture) {
    const Piece *targetPiece = board_.getPieceAt(move.to);
    if (targetPiece != nullptr) {
      const Piece *sourcePiece = board_.getPieceAt(move.from);
      isCapture = (sourcePiece == nullptr) ||
                  (targetPiece->getColor() != sourcePiece->getColor());
    }
  }

  // 3) Execute the move on the board
  board_.executeMove(move);

  // 4) Update castling rights (must be done after move execution
  //    because we need to inspect the moved piece at its new position)
  updateCastlingRights(move);

  // 5) Record the move
  moveHistory_.push(move);

  const ChessColor movingColor = currentTurn_;

  // 6) Switch turn
  currentTurn_ = oppositeColor(currentTurn_);

  // 7) Detect check / checkmate / stalemate for the next player
  updateGameState();

  notify({GameEventType::MoveMade, move.from, move.to, isCapture, movingColor});
  if (state_ == GameState::Check) {
    notify({GameEventType::Check, {}, {}, false, currentTurn_});
  } else if (state_ == GameState::Checkmate) {
    notify({GameEventType::Checkmate, {}, {}, false, currentTurn_});
  } else if (state_ == GameState::Stalemate) {
    notify({GameEventType::Stalemate, {}, {}, false, currentTurn_});
  } else if (state_ == GameState::Draw) {
    notify({GameEventType::Draw, {}, {}, false, currentTurn_});
  }

  return true;
}

void Game::restart() {
  board_.clear();
  board_.setupInitialPosition();
  currentTurn_ = ChessColor::White;
  state_ = GameState::Playing;
  moveHistory_.clear();
  undoStack_.clear();
  capturedWhitePieces_.clear();
  capturedBlackPieces_.clear();
  castlingRights_[0] = true;
  castlingRights_[1] = true;
  castlingRights_[2] = true;
  castlingRights_[3] = true;
  setTimeControl(timeControlMinutes_);
}

void Game::setTimeControl(int minutes) {
  timeControlMinutes_ = minutes;
  whiteTimeLeft_ = minutes * 60.0f;
  blackTimeLeft_ = minutes * 60.0f;
}

void Game::tickTimer(float dt) {
  if (state_ != GameState::Playing) return;
  if (moveHistory_.isEmpty()) return; // Đợi first move thì mới bắt đầu đếm ngược

  if (currentTurn_ == ChessColor::White) {
    whiteTimeLeft_ -= dt;
    if (whiteTimeLeft_ <= 0.0f) {
      whiteTimeLeft_ = 0.0f;
      state_ = GameState::Timeout;
      notify({GameEventType::Checkmate, {}, {}, false, currentTurn_}); // use checkmate event to trigger end screen
    }
  } else {
    blackTimeLeft_ -= dt;
    if (blackTimeLeft_ <= 0.0f) {
      blackTimeLeft_ = 0.0f;
      state_ = GameState::Timeout;
      notify({GameEventType::Checkmate, {}, {}, false, currentTurn_});
    }
  }
}

// ─── Persistence and Undo ───────────────────────────────────────────────────

bool Game::saveGame(const std::string &filename) const {
  std::ofstream out(filename, std::ios::binary);
  if (!out)
    return false;

  int version = 2; // version
  out.write(reinterpret_cast<const char *>(&version), sizeof(version));
  out.write(reinterpret_cast<const char *>(&timeControlMinutes_), sizeof(timeControlMinutes_));
  out.write(reinterpret_cast<const char *>(&whiteTimeLeft_), sizeof(whiteTimeLeft_));
  out.write(reinterpret_cast<const char *>(&blackTimeLeft_), sizeof(blackTimeLeft_));

  size_t numMoves = moveHistory_.getSize();
  out.write(reinterpret_cast<const char *>(&numMoves), sizeof(numMoves));

  for (const auto &move : moveHistory_.toVector()) {
    out.write(reinterpret_cast<const char *>(&move), sizeof(Move));
  }

  return true;
}

bool Game::loadGame(const std::string &filename) {
  std::ifstream in(filename, std::ios::binary);
  if (!in)
    return false;

  int version = 0;
  if (!in.read(reinterpret_cast<char *>(&version), sizeof(version)))
    return false;

  int savedControl = 5;
  float savedWhite = 300.0f;
  float savedBlack = 300.0f;
  size_t numMoves = 0;

  if (version == 2) {
    in.read(reinterpret_cast<char *>(&savedControl), sizeof(savedControl));
    in.read(reinterpret_cast<char *>(&savedWhite), sizeof(savedWhite));
    in.read(reinterpret_cast<char *>(&savedBlack), sizeof(savedBlack));
    if (!in.read(reinterpret_cast<char *>(&numMoves), sizeof(numMoves)))
      return false;
  } else {
    // Old version, version is actually numMoves
    numMoves = static_cast<size_t>(version);
  }

  std::vector<Move> loadedMoves;
  for (size_t i = 0; i < numMoves; ++i) {
    Move move;
    if (!in.read(reinterpret_cast<char *>(&move), sizeof(Move)))
      return false;
    loadedMoves.push_back(move);
  }

  // Prevent UI updates and sounds during replay
  auto savedObservers = observers_;
  observers_.clear();

  restart();
  
  if (version == 2) {
    timeControlMinutes_ = savedControl;
    whiteTimeLeft_ = savedWhite;
    blackTimeLeft_ = savedBlack;
  }

  for (const auto &move : loadedMoves) {
    makeMove(move);
  }

  observers_ = savedObservers;
  notify({GameEventType::GameLoaded, {}, {}, false, currentTurn_});
  return true;
}

bool Game::undo() {
  if (moveHistory_.isEmpty() || undoStack_.empty())
    return false;

  moveHistory_.pop();
  
  GameStateSnapshot snapshot = undoStack_.back();
  undoStack_.pop_back();

  board_ = snapshot.board;
  currentTurn_ = snapshot.currentTurn;
  state_ = snapshot.state;
  castlingRights_[0] = snapshot.castlingRights[0];
  castlingRights_[1] = snapshot.castlingRights[1];
  castlingRights_[2] = snapshot.castlingRights[2];
  castlingRights_[3] = snapshot.castlingRights[3];
  whiteTimeLeft_ = snapshot.whiteTimeLeft;
  blackTimeLeft_ = snapshot.blackTimeLeft;
  capturedWhitePieces_ = snapshot.capturedWhitePieces;
  capturedBlackPieces_ = snapshot.capturedBlackPieces;

  notify({GameEventType::GameLoaded, {}, {}, false, currentTurn_});
  return true;
}

void Game::attach(Observer *observer) {
  if (observer == nullptr) {
    return;
  }

  const auto it = std::find(observers_.begin(), observers_.end(), observer);
  if (it == observers_.end()) {
    observers_.push_back(observer);
  }
}

void Game::detach(Observer *observer) {
  observers_.erase(std::remove(observers_.begin(), observers_.end(), observer),
                   observers_.end());
}

void Game::notify(const GameEvent &event) {
  for (auto *observer : observers_) {
    if (observer != nullptr) {
      observer->update(event);
    }
  }
}
