#ifndef MOVESTACK_H
#define MOVESTACK_H

#include "chess/model/Move.h"
#include <vector>

struct MoveNode {
  Move data;
  MoveNode *next;
  MoveNode(const Move &m, MoveNode *n = nullptr) : data(m), next(n) {}
};

class MoveStack {
private:
  MoveNode *head_;
  int size_;

public:
  MoveStack();
  ~MoveStack();

  // Rule of three
  MoveStack(const MoveStack &other);
  MoveStack &operator=(const MoveStack &other);

  void push(const Move &move);
  Move pop();
  Move peek() const;
  bool isEmpty() const;
  int getSize() const;
  void clear();

  // Helper to return moves from bottom to top (for AI replay & Save/Load)
  std::vector<Move> toVector() const;
};

#endif // MOVESTACK_H
