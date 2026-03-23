#include "ChessSound.h"
#include <string>
#include <vector>

ChessSound::~ChessSound() { unloadSounds(); }

bool ChessSound::loadSounds() {
  const char *appDir = GetApplicationDirectory();
  const std::vector<std::string> moveSoundCandidates = {
      "assets/sounds/piece_move.mp3",
      "../assets/sounds/piece_move.mp3",
      "../../assets/sounds/piece_move.mp3",
      (appDir ? std::string(appDir) : std::string()) +
          "assets/sounds/piece_move.mp3",
      (appDir ? std::string(appDir) : std::string()) +
          "../assets/sounds/piece_move.mp3",
      (appDir ? std::string(appDir) : std::string()) +
          "../../assets/sounds/piece_move.mp3"};

  for (const auto &path : moveSoundCandidates) {
    if (path.empty() || !FileExists(path.c_str())) {
      continue;
    }
    pieceMoveSound_ = LoadSound(path.c_str());
    if (pieceMoveSound_.frameCount > 0) {
      SetSoundVolume(pieceMoveSound_, kMoveSoundVolume_);
      pieceMoveSoundLoaded_ = true;
      break;
    }
  }

  const std::vector<std::string> captureSoundCandidates = {
      "assets/sounds/capture.mp3",
      "../assets/sounds/capture.mp3",
      "../../assets/sounds/capture.mp3",
      (appDir ? std::string(appDir) : std::string()) +
          "assets/sounds/capture.mp3",
      (appDir ? std::string(appDir) : std::string()) +
          "../assets/sounds/capture.mp3",
      (appDir ? std::string(appDir) : std::string()) +
          "../../assets/sounds/capture.mp3"};

  for (const auto &path : captureSoundCandidates) {
    if (path.empty() || !FileExists(path.c_str())) {
      continue;
    }
    captureSound_ = LoadSound(path.c_str());
    if (captureSound_.frameCount > 0) {
      SetSoundVolume(captureSound_, kCaptureSoundVolume_);
      captureSoundLoaded_ = true;
      break;
    }
  }

  return pieceMoveSoundLoaded_ || captureSoundLoaded_;
}

void ChessSound::onMoveMade(Position from, Position to, bool isCapture) {
  (void)from;
  (void)to;

  if (isCapture) {
    if (captureSoundLoaded_) {
      PlaySound(captureSound_);
    } else if (pieceMoveSoundLoaded_) {
      PlaySound(pieceMoveSound_);
    }
    return;
  }
  if (pieceMoveSoundLoaded_) {
    PlaySound(pieceMoveSound_);
  }
}

void ChessSound::unloadSounds() {
  if (pieceMoveSoundLoaded_) {
    UnloadSound(pieceMoveSound_);
    pieceMoveSoundLoaded_ = false;
  }
  if (captureSoundLoaded_) {
    UnloadSound(captureSound_);
    captureSoundLoaded_ = false;
  }
}
