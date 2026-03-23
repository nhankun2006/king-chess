#ifndef CHESSSOUND_H
#define CHESSSOUND_H

#ifdef Color
#undef Color
#endif

#include "Observer.h"

#define Color RLColor
#include <raylib.h>
#undef Color

class ChessSound : public Observer {
private:
  Sound pieceMoveSound_{};
  bool pieceMoveSoundLoaded_ = false;
  Sound captureSound_{};
  bool captureSoundLoaded_ = false;
  static constexpr float kMoveSoundVolume_ = 0.62f;
  static constexpr float kCaptureSoundVolume_ = 5.00f;

public:
  ChessSound() = default;
  ~ChessSound() override;

  bool loadSounds();
  void unloadSounds();

  // Observer interface
  void onMoveMade(Position from, Position to, bool isCapture) override;
  void onCheck(Color color) override { (void)color; }
  void onCheckmate(Color color) override { (void)color; }
  void onStalemate() override {}
  void onDraw() override {}
};

#endif // CHESSSOUND_H
