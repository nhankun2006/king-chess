#ifndef NETWORKPLAYERAGENT_H
#define NETWORKPLAYERAGENT_H

#include <optional>

#include "chess/net/NetworkSession.h"
#include "chess/players/IPlayerAgent.h"

class Game;

/// A proxy player agent that receives moves from a remote peer over TCP.
/// Implements the Proxy pattern: the Controller calls chooseMove() each frame
/// and gets std::nullopt until a Move message arrives from the network,
/// at which point it returns the decoded move — exactly like HumanPlayerAgent
/// returns nullopt until the user clicks.
class NetworkPlayerAgent : public IPlayerAgent {
public:
  NetworkPlayerAgent(ChessColor color, NetworkSession &session);

  ChessColor color() const override;

  /// Returns true — the Controller should not process mouse clicks for
  /// this player's turn (similar to BotPlayerAgent).
  bool isAutomated() const override;

  /// Polls the NetworkSession for an incoming Move message.
  /// Returns std::nullopt if no move has arrived yet (non-blocking).
  std::optional<Move> chooseMove(const Game &game) override;

private:
  ChessColor color_;
  NetworkSession &session_;
};

#endif // NETWORKPLAYERAGENT_H
