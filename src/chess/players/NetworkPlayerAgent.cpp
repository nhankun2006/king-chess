#include "chess/players/NetworkPlayerAgent.h"

#include "chess/model/Game.h"

NetworkPlayerAgent::NetworkPlayerAgent(ChessColor color,
                                       NetworkSession &session)
    : color_(color), session_(session) {}

ChessColor NetworkPlayerAgent::color() const { return color_; }

bool NetworkPlayerAgent::isAutomated() const { return true; }

std::optional<Move> NetworkPlayerAgent::chooseMove(const Game &game) {
  (void)game;

  auto msg = session_.tryReceive();
  if (!msg) return std::nullopt;

  if (msg->type == Protocol::MessageType::Move) {
    return Protocol::decodeMovePayload(msg->payload);
  }

  // Non-move messages (Resign, Ping, etc.) are currently ignored.
  // Future phases can handle them here.
  return std::nullopt;
}
