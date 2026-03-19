#ifndef DRAWER_H
#define DRAWER_H

#include <map>

#include "Types.h"

#define Color RLColor
#include <raylib.h>
#undef Color

#include "Board.h"

class Drawer {
private:
  Texture2D boardTexture_{};
  std::map<PieceType, Texture2D> whiteTextures_;
  std::map<PieceType, Texture2D> blackTextures_;

public:
  Drawer() = default;

  ~Drawer() {
    if (boardTexture_.id != 0) {
      UnloadTexture(boardTexture_);
    }

    for (auto &entry : whiteTextures_) {
      if (entry.second.id != 0) {
        UnloadTexture(entry.second);
      }
    }
    for (auto &entry : blackTextures_) {
      if (entry.second.id != 0) {
        UnloadTexture(entry.second);
      }
    }
  }

  Texture2D LoadImage(const char *path);
  bool LoadAssets();
  void drawPiece(PieceType type, Color color, float x, float y, float w,
                 float h);
  void drawBoard(const Board &board);
};

inline Texture2D Drawer::LoadImage(const char *path) { return LoadTexture(path); }

inline bool Drawer::LoadAssets() {
  boardTexture_ = LoadImage("assets/images/board1.png");
  if (boardTexture_.id == 0) {
    return false;
  }

  whiteTextures_[PieceType::Pawn] = LoadImage("assets/images/W_Pawn.png");
  whiteTextures_[PieceType::Knight] = LoadImage("assets/images/W_Knight.png");
  whiteTextures_[PieceType::Bishop] = LoadImage("assets/images/W_Bishop.png");
  whiteTextures_[PieceType::Rook] = LoadImage("assets/images/W_Rook.png");
  whiteTextures_[PieceType::Queen] = LoadImage("assets/images/W_Queen.png");
  whiteTextures_[PieceType::King] = LoadImage("assets/images/W_King.png");

  blackTextures_[PieceType::Pawn] = LoadImage("assets/images/B_Pawn.png");
  blackTextures_[PieceType::Knight] = LoadImage("assets/images/B_Knight.png");
  blackTextures_[PieceType::Bishop] = LoadImage("assets/images/B_Bishop.png");
  blackTextures_[PieceType::Rook] = LoadImage("assets/images/B_Rook.png");
  blackTextures_[PieceType::Queen] = LoadImage("assets/images/B_Queen.png");
  blackTextures_[PieceType::King] = LoadImage("assets/images/B_King.png");

  for (const auto &entry : whiteTextures_) {
    if (entry.second.id == 0) {
      return false;
    }
  }
  for (const auto &entry : blackTextures_) {
    if (entry.second.id == 0) {
      return false;
    }
  }

  return true;
}

inline void Drawer::drawPiece(PieceType type, Color color, float x, float y,
                              float w, float h) {
  const auto &textures = (color == Color::White) ? whiteTextures_ : blackTextures_;
  const auto it = textures.find(type);
  if (it == textures.end() || it->second.id == 0) {
    return;
  }

  const Texture2D &texture = it->second;
  const Rectangle src = {0.0f, 0.0f, static_cast<float>(texture.width),
                         static_cast<float>(texture.height)};
  const Rectangle dst = {x, y, w, h};
  DrawTexturePro(texture, src, dst, {0.0f, 0.0f}, 0.0f, {255, 255, 255, 255});
}

inline void Drawer::drawBoard(const Board &board) {
  BeginDrawing();
  ClearBackground({0, 0, 0, 255});

  if (boardTexture_.id != 0) {
    const Rectangle src = {0.0f, 0.0f, static_cast<float>(boardTexture_.width),
                           static_cast<float>(boardTexture_.height)};
    const Rectangle dst = {0.0f, 0.0f, 512.0f, 512.0f};
    DrawTexturePro(boardTexture_, src, dst, {0.0f, 0.0f}, 0.0f,
                   {255, 255, 255, 255});
  }

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      const Piece *piece = board.getPieceAt({row, col});
      if (piece) {
        drawPiece(piece->getType(), piece->getColor(), static_cast<float>(col * 64),
                  static_cast<float>(row * 64), 64.0f, 64.0f);
      }
    }
  }

  EndDrawing();
}

#endif // DRAWER_H
