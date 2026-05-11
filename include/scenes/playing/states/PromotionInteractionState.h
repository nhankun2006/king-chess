#ifndef PROMOTIONINTERACTIONSTATE_H
#define PROMOTIONINTERACTIONSTATE_H

#include "scenes/playing/ChessControllerState.h"
#include "chess/model/Move.h"
#include "chess/model/Types.h"

#include <vector>

class PromotionInteractionState : public ChessControllerState {
public:
  PromotionInteractionState(std::vector<Move> promotionMoves,
                            ChessColor color);

  bool handleInput(ChessController &ctrl) override;

private:
  std::vector<Move> pendingMoves_;
  ChessColor color_;
};

#endif // PROMOTIONINTERACTIONSTATE_H
