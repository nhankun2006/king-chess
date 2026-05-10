#include "BotAI.h"

#include <algorithm>
#include <limits>

namespace {

constexpr int kPawnTable[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    {5, 5, 10, 25, 25, 10, 5, 5},
    {0, 0, 0, 20, 20, 0, 0, 0},
    {5, -5, -10, 0, 0, -10, -5, 5},
    {5, 10, 10, -20, -20, 10, 10, 5},
    {0, 0, 0, 0, 0, 0, 0, 0},
};

constexpr int kKnightTable[8][8] = {
    {-50, -40, -30, -30, -30, -30, -40, -50},
    {-40, -20, 0, 5, 5, 0, -20, -40},
    {-30, 5, 10, 15, 15, 10, 5, -30},
    {-30, 0, 15, 20, 20, 15, 0, -30},
    {-30, 5, 15, 20, 20, 15, 5, -30},
    {-30, 0, 10, 15, 15, 10, 0, -30},
    {-40, -20, 0, 0, 0, 0, -20, -40},
    {-50, -40, -30, -30, -30, -30, -40, -50},
};

constexpr int kBishopTable[8][8] = {
    {-20, -10, -10, -10, -10, -10, -10, -20},
    {-10, 5, 0, 0, 0, 0, 5, -10},
    {-10, 10, 10, 10, 10, 10, 10, -10},
    {-10, 0, 10, 10, 10, 10, 0, -10},
    {-10, 5, 5, 10, 10, 5, 5, -10},
    {-10, 0, 5, 10, 10, 5, 0, -10},
    {-10, 0, 0, 0, 0, 0, 0, -10},
    {-20, -10, -10, -10, -10, -10, -10, -20},
};

constexpr int kRookTable[8][8] = {
    {0, 0, 0, 5, 5, 0, 0, 0},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {5, 10, 10, 10, 10, 10, 10, 5},
    {0, 0, 0, 0, 0, 0, 0, 0},
};

constexpr int kQueenTable[8][8] = {
    {-20, -10, -10, -5, -5, -10, -10, -20},
    {-10, 0, 5, 0, 0, 0, 0, -10},
    {-10, 5, 5, 5, 5, 5, 0, -10},
    {0, 0, 5, 5, 5, 5, 0, -5},
    {-5, 0, 5, 5, 5, 5, 0, -5},
    {-10, 0, 5, 5, 5, 5, 0, -10},
    {-10, 0, 0, 0, 0, 0, 0, -10},
    {-20, -10, -10, -5, -5, -10, -10, -20},
};

constexpr int kKingMidgameTable[8][8] = {
    {20, 30, 10, 0, 0, 10, 30, 20},
    {20, 20, 0, 0, 0, 0, 20, 20},
    {-10, -20, -20, -20, -20, -20, -20, -10},
    {-20, -30, -30, -40, -40, -30, -30, -20},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
};

constexpr int kKingEndgameTable[8][8] = {
    {-50, -30, -30, -30, -30, -30, -30, -50},
    {-30, -30, 0, 0, 0, 0, -30, -30},
    {-30, -10, 20, 30, 30, 20, -10, -30},
    {-30, -10, 30, 40, 40, 30, -10, -30},
    {-30, -10, 30, 40, 40, 30, -10, -30},
    {-30, -10, 20, 30, 30, 20, -10, -30},
    {-30, -20, -10, 0, 0, -10, -20, -30},
    {-50, -40, -30, -20, -20, -30, -40, -50},
};

int pieceSquareValue(PieceType pieceType, ChessColor color, Position pos,
                     bool isEndgame) {
  const int row = (color == ChessColor::White) ? pos.row : (7 - pos.row);
  const int col = pos.col;

  switch (pieceType) {
  case PieceType::Pawn:
    return kPawnTable[row][col];
  case PieceType::Knight:
    return kKnightTable[row][col];
  case PieceType::Bishop:
    return kBishopTable[row][col];
  case PieceType::Rook:
    return kRookTable[row][col];
  case PieceType::Queen:
    return kQueenTable[row][col];
  case PieceType::King:
    return isEndgame ? kKingEndgameTable[row][col] : kKingMidgameTable[row][col];
  case PieceType::None:
    break;
  }

  return 0;
}

PieceType getCapturedPieceType(const Game &node, const Move &move) {
  const Board &board = node.getBoard();
  if (move.isEnPassant) {
    const Position capturedPawnPos = {move.from.row, move.to.col};
    const Piece *captured = board.getPieceAt(capturedPawnPos);
    return (captured == nullptr) ? PieceType::None : captured->getType();
  }

  const Piece *captured = board.getPieceAt(move.to);
  return (captured == nullptr) ? PieceType::None : captured->getType();
}

} // namespace

Move BotAI::chooseBestMove(const Game &liveGame) const {
  Game root = buildSearchRoot(liveGame);
  if (root.getCurrentTurn() != botColor_) {
    return {{-1, -1}, {-1, -1}};
  }

  std::vector<Move> legalMoves = root.getAllLegalMoves(root.getCurrentTurn());
  if (legalMoves.empty()) {
    return {{-1, -1}, {-1, -1}};
  }

  orderMoves(root, legalMoves);

  int alpha = std::numeric_limits<int>::min();
  int beta = std::numeric_limits<int>::max();
  int bestScore = std::numeric_limits<int>::min();
  Move bestMove = legalMoves.front();
  bool hasBestMove = false;

  for (const Move &move : legalMoves) {
    Game child(root);
    if (!child.makeMove(move)) {
      continue;
    }

    const int score = alphaBeta(child, kFixedDepth_ - 1, alpha, beta, 1);
    if (score > bestScore) {
      bestScore = score;
      bestMove = move;
      hasBestMove = true;
    }

    alpha = std::max(alpha, bestScore);
    if (alpha >= beta) {
      break;
    }
  }

  if (!hasBestMove) {
    return {{-1, -1}, {-1, -1}};
  }

  return bestMove;
}

Game BotAI::buildSearchRoot(const Game &liveGame) const {
  Game searchRoot;
  const std::vector<Move> history = liveGame.getMoveHistory();
  for (const Move &move : history) {
    if (!searchRoot.makeMove(move)) {
      break;
    }
  }
  return searchRoot;
}

int BotAI::alphaBeta(Game &node, int depth, int alpha, int beta, int ply) const {
  const GameState state = node.getState();
  if (state == GameState::Checkmate) {
    return (node.getCurrentTurn() == botColor_) ? (-kMateScore_ + ply)
                                                : (kMateScore_ - ply);
  }
  if (state == GameState::Stalemate || state == GameState::Draw) {
    return 0;
  }
  if (depth == 0) {
    return evaluate(node);
  }

  std::vector<Move> moves = node.getAllLegalMoves(node.getCurrentTurn());
  if (moves.empty()) {
    if (node.getBoard().isInCheck(node.getCurrentTurn())) {
      return (node.getCurrentTurn() == botColor_) ? (-kMateScore_ + ply)
                                                  : (kMateScore_ - ply);
    }
    return 0;
  }

  orderMoves(node, moves);

  const bool maximizing = (node.getCurrentTurn() == botColor_);
  if (maximizing) {
    int bestScore = std::numeric_limits<int>::min();
    bool hasScore = false;
    for (const Move &move : moves) {
      Game child(node);
      if (!child.makeMove(move)) {
        continue;
      }

      const int score = alphaBeta(child, depth - 1, alpha, beta, ply + 1);
      hasScore = true;
      bestScore = std::max(bestScore, score);
      alpha = std::max(alpha, bestScore);
      if (alpha >= beta) {
        break;
      }
    }
    if (!hasScore) {
      return evaluate(node);
    }
    return bestScore;
  }

  int bestScore = std::numeric_limits<int>::max();
  bool hasScore = false;
  for (const Move &move : moves) {
    Game child(node);
    if (!child.makeMove(move)) {
      continue;
    }

    const int score = alphaBeta(child, depth - 1, alpha, beta, ply + 1);
    hasScore = true;
    bestScore = std::min(bestScore, score);
    beta = std::min(beta, bestScore);
    if (alpha >= beta) {
      break;
    }
  }
  if (!hasScore) {
    return evaluate(node);
  }
  return bestScore;
}

int BotAI::evaluate(const Game &node) const {
  const Board &board = node.getBoard();
  const bool endgame = isEndgamePosition(board);

  int score = 0;
  int botBishopCount = 0;
  int oppBishopCount = 0;

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      const Position pos = {row, col};
      const Piece *piece = board.getPieceAt(pos);
      if (piece == nullptr) {
        continue;
      }

      const PieceType type = piece->getType();
      const ChessColor color = piece->getColor();
      const int material = pieceValue(type);
      const int positional = pieceSquareValue(type, color, pos, endgame);
      const int pieceScore = material + positional;

      if (type == PieceType::Bishop) {
        if (color == botColor_) {
          ++botBishopCount;
        } else {
          ++oppBishopCount;
        }
      }

      if (color == botColor_) {
        score += pieceScore;
      } else {
        score -= pieceScore;
      }
    }
  }

  if (botBishopCount >= 2) {
    score += 30;
  }
  if (oppBishopCount >= 2) {
    score -= 30;
  }

  if (node.getState() == GameState::Check) {
    if (node.getCurrentTurn() == botColor_) {
      score -= 35;
    } else {
      score += 35;
    }
  }

  return score;
}

void BotAI::orderMoves(const Game &node, std::vector<Move> &moves) const {
  std::stable_sort(moves.begin(), moves.end(), [&](const Move &a, const Move &b) {
    return scoreMoveHeuristic(node, a) > scoreMoveHeuristic(node, b);
  });
}

int BotAI::scoreMoveHeuristic(const Game &node, const Move &move) const {
  const Board &board = node.getBoard();
  const Piece *mover = board.getPieceAt(move.from);

  int score = 0;

  const PieceType capturedType = getCapturedPieceType(node, move);
  if (capturedType != PieceType::None) {
    const int capturedValue = pieceValue(capturedType);
    const int moverValue = (mover == nullptr) ? 0 : pieceValue(mover->getType());
    score += 10000 + (capturedValue * 10 - moverValue);
  }

  if (move.promotion != PieceType::None) {
    score += 8000 + pieceValue(move.promotion);
  }

  if (move.isCastling) {
    score += 500;
  }
  if (move.isEnPassant) {
    score += 250;
  }

  return score;
}

int BotAI::pieceValue(PieceType pieceType) {
  switch (pieceType) {
  case PieceType::Pawn:
    return 100;
  case PieceType::Knight:
    return 320;
  case PieceType::Bishop:
    return 330;
  case PieceType::Rook:
    return 500;
  case PieceType::Queen:
    return 900;
  case PieceType::King:
    return 0;
  case PieceType::None:
    break;
  }
  return 0;
}

bool BotAI::isEndgamePosition(const Board &board) {
  int nonPawnMaterial = 0;
  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      const Piece *piece = board.getPieceAt({row, col});
      if (piece == nullptr) {
        continue;
      }

      const PieceType type = piece->getType();
      if (type == PieceType::Pawn || type == PieceType::King) {
        continue;
      }

      nonPawnMaterial += pieceValue(type);
    }
  }

  return nonPawnMaterial <= 2600;
}
