#include <SDL3/SDL.h>
#include <map>
#include "Types.h"
#include "Board.h"
// #include <SDL3/SDL_image.h>

class Drawer {
private:
  SDL_Renderer *renderer_;
  SDL_Texture *boardTexture_;
  std::map<PieceType, SDL_Texture *> whiteTextures_;
  std::map<PieceType, SDL_Texture *> blackTextures_;

public:
  Drawer(SDL_Renderer *renderer) : renderer_(renderer) {}
  ~Drawer() {
    SDL_DestroyTexture(boardTexture_);
    for (auto &texture : whiteTextures_) {
      SDL_DestroyTexture(texture.second);
    }
    for (auto &texture : blackTextures_) {
      SDL_DestroyTexture(texture.second);
    }
    SDL_DestroyRenderer(renderer_);
  };

  SDL_Texture *LoadImage(const char *path);
  bool LoadAssets();
  void drawPiece(PieceType type, Color color, float x, float y, float w,
                 float h);
  void drawBoard(const Board &board);
};

SDL_Texture *Drawer::LoadImage(const char *path) {
  SDL_Surface *surface = nullptr; // IMG_Load(path);
  //  install SDL_image later
  if (!surface) {
    return nullptr;
  }
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer_, surface);
  if (!texture) {
    return nullptr;
  }
  return texture;
}

bool Drawer::LoadAssets() {
  boardTexture_ = LoadImage("assets/images/board1.png");
  if (!boardTexture_) {
    return false;
  }
  whiteTextures_[PieceType::Pawn] = LoadImage("assets/images/W_pawn.png");
  whiteTextures_[PieceType::Knight] = LoadImage("assets/images/W_knight.png");
  whiteTextures_[PieceType::Bishop] = LoadImage("assets/images/W_bishop.png");
  whiteTextures_[PieceType::Rook] = LoadImage("assets/images/W_rook.png");
  whiteTextures_[PieceType::Queen] = LoadImage("assets/images/W_queen.png");
  whiteTextures_[PieceType::King] = LoadImage("assets/images/W_king.png");
  blackTextures_[PieceType::Pawn] = LoadImage("assets/images/B_pawn.png");
  blackTextures_[PieceType::Knight] = LoadImage("assets/images/B_knight.png");
  blackTextures_[PieceType::Bishop] = LoadImage("assets/images/B_bishop.png");
  blackTextures_[PieceType::Rook] = LoadImage("assets/images/B_rook.png");
  blackTextures_[PieceType::Queen] = LoadImage("assets/images/B_queen.png");
  blackTextures_[PieceType::King] = LoadImage("assets/images/B_king.png");
  return true;
}

void Drawer::drawPiece(PieceType type, Color color, float x, float y, float w,
                       float h) {
  SDL_Texture *texture = nullptr;
  if (color == Color::White) {
    texture = whiteTextures_[type];
  } else {
    texture = blackTextures_[type];
  }
  if (texture) {
    SDL_FRect dest = {x, y, w, h};
    SDL_RenderTexture(renderer_, texture, nullptr, &dest);
  }
}

void Drawer::drawBoard(const Board &board) {
  SDL_RenderClear(renderer_);
  SDL_FRect dest_BoardLayer = {0, 0, 512, 512};
  SDL_RenderTexture(renderer_, boardTexture_, nullptr, &dest_BoardLayer);
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      const Piece *piece = board.getPieceAt({x, y});
      if (piece) {
        drawPiece(piece->getType(), piece->getColor(), x, y, 64, 64);
      }
    }
  }
  SDL_RenderPresent(renderer_);
}
