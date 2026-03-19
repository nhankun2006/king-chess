#ifndef OBSERVER_H
#define OBSERVER_H

#include "Types.h"

class Observer {
public:
  virtual ~Observer() = default;
  virtual void onMoveMade(Position from, Position to) = 0;
  virtual void onCheck(Color color) = 0;
  virtual void onCheckmate(Color color) = 0;
  virtual void onStalemate() = 0;
  virtual void onDraw() = 0;
};

#endif // OBSERVER_H