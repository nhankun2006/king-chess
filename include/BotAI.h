#ifndef BOTAI_H
#define BOTAI_H

#include <vector>

#include "Game.h"

class BotAI {
private:
  static constexpr int kFixedDepth_ = 4;
  static constexpr int kMateScore_ = 100000;

  ChessColor botColor_ = ChessColor::Black;

  Game buildSearchRoot(const Game &liveGame) const;
  int alphaBeta(Game &node, int depth, int alpha, int beta, int ply) const;
  int evaluate(const Game &node) const;
  void orderMoves(const Game &node, std::vector<Move> &moves) const;
  int scoreMoveHeuristic(const Game &node, const Move &move) const;

  static int pieceValue(PieceType pieceType);
  static bool isEndgamePosition(const Board &board);

public:
  explicit BotAI(ChessColor botColor = ChessColor::Black)
      : botColor_(botColor) {}

  Move chooseBestMove(const Game &liveGame) const;
};

#endif // BOTAI_H
