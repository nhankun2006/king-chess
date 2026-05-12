#ifndef IPLAYERAGENT_H
#define IPLAYERAGENT_H

#include <optional>

#include "chess/model/Move.h"
#include "chess/model/Types.h"

class Game;

class IPlayerAgent {
public:
  virtual ~IPlayerAgent() = default;

  virtual ChessColor color() const = 0;
  virtual bool isAutomated() const = 0;
  virtual std::optional<Move> chooseMove(const Game &game) = 0;
};

#endif // IPLAYERAGENT_H
