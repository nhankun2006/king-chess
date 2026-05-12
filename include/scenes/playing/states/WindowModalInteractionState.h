#ifndef WINDOWMODALINTERACTIONSTATE_H
#define WINDOWMODALINTERACTIONSTATE_H

#include "scenes/playing/ChessControllerState.h"

class WindowModalInteractionState : public ChessControllerState {
public:
  bool handleInput(ChessController &ctrl) override;
  bool isWindowSizeDialogOpen() const override { return true; }
  bool blocksBotInput() const override { return true; }
};

#endif // WINDOWMODALINTERACTIONSTATE_H
