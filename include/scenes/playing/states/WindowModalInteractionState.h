#ifndef WINDOWMODALINTERACTIONSTATE_H
#define WINDOWMODALINTERACTIONSTATE_H

#include "scenes/playing/ChessControllerState.h"

class WindowModalInteractionState : public ChessControllerState {
public:
  bool handleInput(ChessController &ctrl) override;
};

#endif // WINDOWMODALINTERACTIONSTATE_H
