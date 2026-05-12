#include "chess/players/PlayerFactory.h"

#include <memory>

#include "chess/players/BotPlayerAgent.h"
#include "chess/players/HumanPlayerAgent.h"

PlayerFactory::PlayerPair PlayerFactory::create(PlayMode mode) {
  switch (mode) {
  case PlayMode::PvP:
    return {std::make_unique<HumanPlayerAgent>(ChessColor::White),
            std::make_unique<HumanPlayerAgent>(ChessColor::Black)};

  case PlayMode::PvE:
    return {std::make_unique<HumanPlayerAgent>(ChessColor::White),
            std::make_unique<BotPlayerAgent>(ChessColor::Black)};

  case PlayMode::EvE:
    return {std::make_unique<BotPlayerAgent>(ChessColor::White),
            std::make_unique<BotPlayerAgent>(ChessColor::Black)};
  }

  return {std::make_unique<HumanPlayerAgent>(ChessColor::White),
          std::make_unique<HumanPlayerAgent>(ChessColor::Black)};
}
