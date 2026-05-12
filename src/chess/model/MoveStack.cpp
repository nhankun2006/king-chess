#include "chess/model/MoveStack.h"
#include <algorithm>

MoveStack::MoveStack() : head_(nullptr), size_(0) {}

MoveStack::~MoveStack() { clear(); }

MoveStack::MoveStack(const MoveStack &other) : head_(nullptr), size_(0) {
  if (!other.head_)
    return;
  // Copy the stack keeping order
  std::vector<Move> moves = other.toVector();
  for (const auto &m : moves) {
    push(m);
  }
}

MoveStack &MoveStack::operator=(const MoveStack &other) {
  if (this == &other)
    return *this;
  clear();
  std::vector<Move> moves = other.toVector();
  for (const auto &m : moves) {
    push(m);
  }
  return *this;
}

void MoveStack::push(const Move &move) {
  head_ = new MoveNode(move, head_);
  size_++;
}

Move MoveStack::pop() {
  if (!head_)
    return Move(); 
  MoveNode *temp = head_;
  Move move = temp->data;
  head_ = head_->next;
  delete temp;
  size_--;
  return move;
}

Move MoveStack::peek() const {
  if (!head_)
    return Move();
  return head_->data;
}

bool MoveStack::isEmpty() const { return head_ == nullptr; }

int MoveStack::getSize() const { return size_; }

void MoveStack::clear() {
  while (head_) {
    MoveNode *temp = head_;
    head_ = head_->next;
    delete temp;
  }
  size_ = 0;
}

std::vector<Move> MoveStack::toVector() const {
  std::vector<Move> moves;
  MoveNode *curr = head_;
  while (curr) {
    moves.push_back(curr->data);
    curr = curr->next;
  }
  // Stack pops from top to bottom, so we reverse it to get bottom to top
  std::reverse(moves.begin(), moves.end());
  return moves;
}
