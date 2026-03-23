#ifndef CHESSSOUND_H
#define CHESSSOUND_H

#ifdef Color
#undef Color
#endif

#define Color RLColor
#include <raylib.h>
#undef Color

class ChessSound {
private:
  Sound pieceMoveSound_{};
  bool pieceMoveSoundLoaded_ = false;
  Sound captureSound_{};
  bool captureSoundLoaded_ = false;
  static constexpr float kMoveSoundVolume_ = 0.62f;
  static constexpr float kCaptureSoundVolume_ = 5.00f;

public:
  ChessSound() = default;
  ~ChessSound();

  bool loadSounds();
  void playMoveSound(bool isCapture);
  void unloadSounds();
};

#endif // CHESSSOUND_H
