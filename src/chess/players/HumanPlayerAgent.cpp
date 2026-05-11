#include "chess/players/HumanPlayerAgent.h"

#include "chess/model/Game.h"

HumanPlayerAgent::HumanPlayerAgent(ChessColor color) : color_(color) {}

ChessColor HumanPlayerAgent::color() const { return color_; }

bool HumanPlayerAgent::isAutomated() const { return false; }

std::optional<Move> HumanPlayerAgent::chooseMove(const Game &game) {
  (void)game;
  return std::nullopt;
}
