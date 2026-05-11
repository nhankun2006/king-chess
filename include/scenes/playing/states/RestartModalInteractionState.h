#ifndef RESTARTMODALINTERACTIONSTATE_H
#define RESTARTMODALINTERACTIONSTATE_H

#include "scenes/playing/ChessControllerState.h"

class RestartModalInteractionState : public ChessControllerState {
public:
  bool handleInput(ChessController &ctrl) override;
};

#endif // RESTARTMODALINTERACTIONSTATE_H
