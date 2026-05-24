#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include <mutex>
#include <optional>
#include <queue>

/// A simple thread-safe FIFO queue.
/// The receive thread pushes messages; the main thread pops them.
/// Uses a std::mutex — perfectly adequate for the low-frequency
/// message traffic of a turn-based chess game.
template <typename T>
class ThreadSafeQueue {
public:
  /// Push a value onto the back of the queue (thread-safe).
  void push(T value) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(std::move(value));
  }

  /// Try to pop a value from the front of the queue.
  /// Returns std::nullopt if the queue is empty (non-blocking).
  std::optional<T> tryPop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) return std::nullopt;
    T value = std::move(queue_.front());
    queue_.pop();
    return value;
  }

  /// Check whether the queue is empty (thread-safe).
  bool empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  /// Clear all pending items (thread-safe).
  void clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::queue<T> empty;
    queue_.swap(empty);
  }

private:
  mutable std::mutex mutex_;
  std::queue<T> queue_;
};

#endif // THREADSAFEQUEUE_H
