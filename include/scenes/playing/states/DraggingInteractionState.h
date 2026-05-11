#ifndef DRAGGINGINTERACTIONSTATE_H
#define DRAGGINGINTERACTIONSTATE_H

#include "scenes/playing/ChessControllerState.h"
#include "chess/model/Types.h"

#include <optional>

class DraggingInteractionState : public ChessControllerState {
public:
  DraggingInteractionState(Position from, PieceType type, ChessColor color);

  bool handleInput(ChessController &ctrl) override;

private:
  Position dragFrom_;
  PieceType dragPieceType_;
  ChessColor dragPieceColor_;
};

#endif // DRAGGINGINTERACTIONSTATE_H
