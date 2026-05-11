#ifndef HUMANPLAYERAGENT_H
#define HUMANPLAYERAGENT_H

#include <optional>

#include "chess/players/IPlayerAgent.h"

class Game;

class HumanPlayerAgent : public IPlayerAgent {
public:
  explicit HumanPlayerAgent(ChessColor color);

  ChessColor color() const override;
  bool isAutomated() const override;
  std::optional<Move> chooseMove(const Game &game) override;

private:
  ChessColor color_;
};

#endif // HUMANPLAYERAGENT_H
