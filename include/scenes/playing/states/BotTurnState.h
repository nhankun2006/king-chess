#ifndef BOTTURNSTATE_H
#define BOTTURNSTATE_H

#include "scenes/playing/ChessControllerState.h"
#include "chess/model/Move.h"

#include <future>
#include <optional>

/// State active while an automated player (bot) is computing its move.
/// The heavy AI work runs on a background thread via std::async so the
/// UI remains responsive.  Each frame we poll the future; once it is
/// ready we apply the move and transition back to IdleInteractionState.
class BotTurnState : public ChessControllerState {
public:
  bool handleInput(ChessController &ctrl) override;

private:
  std::future<std::optional<Move>> future_;
  bool launched_ = false;
};

#endif // BOTTURNSTATE_H
