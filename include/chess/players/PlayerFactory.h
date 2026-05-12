#ifndef PLAYERFACTORY_H
#define PLAYERFACTORY_H

#include <memory>

#include "chess/players/IPlayerAgent.h"
#include "scenes/PlayMode.h"

class PlayerFactory {
public:
  struct PlayerPair {
    std::unique_ptr<IPlayerAgent> white;
    std::unique_ptr<IPlayerAgent> black;
  };

  static PlayerPair create(PlayMode mode);
};

#endif // PLAYERFACTORY_H
