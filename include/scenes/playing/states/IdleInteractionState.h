#ifndef IDLEINTERACTIONSTATE_H
#define IDLEINTERACTIONSTATE_H

#include "scenes/playing/ChessControllerState.h"

class IdleInteractionState : public ChessControllerState {
public:
  bool handleInput(ChessController &ctrl) override;
};

#endif // IDLEINTERACTIONSTATE_H
