#ifndef OBSERVER_H
#define OBSERVER_H

#include "Types.h"

enum class GameEventType { MoveMade, Check, Checkmate, Stalemate, Draw };

struct GameEvent {
  GameEventType type = GameEventType::MoveMade;
  Position from{};
  Position to{};
  bool isCapture = false;
  ChessColor color = ChessColor::White;
};

class Observer {
public:
  virtual ~Observer() = default;
  virtual void update(const GameEvent &event) = 0;
};

#endif // OBSERVER_H
