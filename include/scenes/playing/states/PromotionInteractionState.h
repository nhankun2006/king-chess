#ifndef PROMOTIONINTERACTIONSTATE_H
#define PROMOTIONINTERACTIONSTATE_H

#include "scenes/playing/ChessControllerState.h"
#include "chess/model/Types.h"

/// State active while the promotion dialog is displayed.
/// Stores the from/to squares and delegates piece-type resolution
/// to Game::resolveLegalMove() — the controller never filters moves itself.
class PromotionInteractionState : public ChessControllerState {
public:
  PromotionInteractionState(Position from, Position to, ChessColor color);

  bool handleInput(ChessController &ctrl) override;

private:
  Position from_;
  Position to_;
  ChessColor color_;
};

#endif // PROMOTIONINTERACTIONSTATE_H
