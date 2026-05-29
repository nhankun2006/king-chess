#ifndef NETWORKWAITSTATE_H
#define NETWORKWAITSTATE_H

#include "scenes/playing/ChessControllerState.h"

/// State active while waiting for a remote player's move over the network.
/// Each frame, the controller polls the NetworkPlayerAgent (via the normal
/// automated-player path).  Once chooseMove() returns a valid Move, the
/// controller applies it and transitions back to IdleInteractionState.
///
/// This is the network analogue of BotTurnState.
class NetworkWaitState : public ChessControllerState {
public:
  bool handleInput(ChessController &ctrl) override;

  /// Block bot auto-transition while we are waiting for the remote peer.
  bool blocksBotInput() const override { return true; }

  /// Let the view know we are waiting for the network peer.
  bool isBotThinking() const override { return true; }
};

#endif // NETWORKWAITSTATE_H
