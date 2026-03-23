#ifndef OBSERVER_H
#define OBSERVER_H

#include "Types.h"

class Observer {
public:
  virtual ~Observer() = default;
  virtual void onMoveMade(Position from, Position to, bool isCapture) = 0;
  virtual void onCheck(ChessColor color) = 0;
  virtual void onCheckmate(ChessColor color) = 0;
  virtual void onStalemate() = 0;
  virtual void onDraw() = 0;
};

#endif // OBSERVER_H