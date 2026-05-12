#ifndef RESTARTMODALINTERACTIONSTATE_H
#define RESTARTMODALINTERACTIONSTATE_H

#include "scenes/playing/ChessControllerState.h"

class RestartModalInteractionState : public ChessControllerState {
public:
  bool handleInput(ChessController &ctrl) override;
  bool isRestartModalOpen() const override { return true; }
  bool blocksBotInput() const override { return true; }
};

#endif // RESTARTMODALINTERACTIONSTATE_H
