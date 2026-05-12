#ifndef BOTPLAYERAGENT_H
#define BOTPLAYERAGENT_H

#include <optional>

#include "chess/ai/BotAI.h"
#include "chess/players/IPlayerAgent.h"

class Game;

class BotPlayerAgent : public IPlayerAgent {
public:
  explicit BotPlayerAgent(ChessColor color);

  ChessColor color() const override;
  bool isAutomated() const override;
  std::optional<Move> chooseMove(const Game &game) override;

private:
  ChessColor color_;
  BotAI botAI_;
};

#endif // BOTPLAYERAGENT_H
