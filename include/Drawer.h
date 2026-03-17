#include <SDL3/SDL.h>
#include <map>
#include "Types.h"

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
  };

  SDL_Texture *LoadImage(const char *path);
  bool LoadAssets();
  void drawBoard();
  void drawPiece(PieceType type, Color color, int x, int y, int w, int h);
};

SDL_Texture *Drawer::LoadImage(const char *path) {
  SDL_Surface *surface = IMG_Load(path);
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
  textures_[PieceType::Pawn] = LoadImage("assets/images/pawn.png");
  textures_[PieceType::Knight] = LoadImage("assets/images/knight.png");
  textures_[PieceType::Bishop] = LoadImage("assets/images/bishop.png");
  textures_[PieceType::Rook] = LoadImage("assets/images/rook.png");
  textures_[PieceType::Queen] = LoadImage("assets/images/queen.png");
  textures_[PieceType::King] = LoadImage("assets/images/king.png");
  return true;
}
