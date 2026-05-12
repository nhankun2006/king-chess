#include "chess/players/BotPlayerAgent.h"

#include "chess/model/Game.h"

namespace {

bool isInvalidBotMove(const Move &move) {
  return move.from.row == -1 && move.from.col == -1 && move.to.row == -1 &&
         move.to.col == -1;
}

} // namespace

BotPlayerAgent::BotPlayerAgent(ChessColor color) : color_(color), botAI_(color) {}

ChessColor BotPlayerAgent::color() const { return color_; }

bool BotPlayerAgent::isAutomated() const { return true; }

std::optional<Move> BotPlayerAgent::chooseMove(const Game &game) {
  const Move move = botAI_.chooseBestMove(game);
  if (isInvalidBotMove(move)) {
    return std::nullopt;
  }
  return move;
}
