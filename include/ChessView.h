#ifndef CHESSVIEW_H
#define CHESSVIEW_H

#include <map>
#include <optional>
#include <string>
#include <vector>

#ifdef Color
#undef Color
#endif

#include "Observer.h"
#include "Types.h"

#define Color RLColor
#include <raylib.h>
#undef Color

#include "Board.h"
#include "Move.h"

class ChessView : public Observer {
private:
  Texture2D boardTexture_{};
  std::map<PieceType, Texture2D> whiteTextures_;
  std::map<PieceType, Texture2D> blackTextures_;

public:
  ChessView() = default;

  ~ChessView() {
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

  bool LoadAssets();
  void drawPiece(PieceType type, ::Color color, float x, float y, float w,
                 float h);
  void drawBoard(const Board &board,
                 const std::optional<Position> &selectedSquare = std::nullopt,
                 const std::vector<Move> &legalMoves = {});

  void onMoveMade(Position from, Position to) override {
    (void)from;
    (void)to;
  }

  void onCheck(::Color color) override { (void)color; }

  void onCheckmate(::Color color) override { (void)color; }

  void onStalemate() override {}

  void onDraw() override {}
};

inline bool ChessView::LoadAssets() {
  boardTexture_ = LoadTexture("assets/images/board1.png");
  if (boardTexture_.id == 0) {
    return false;
  }

  static const std::pair<PieceType, std::string> pieces[] = {
      {PieceType::Pawn, "Pawn"},     {PieceType::Knight, "Knight"},
      {PieceType::Bishop, "Bishop"}, {PieceType::Rook, "Rook"},
      {PieceType::Queen, "Queen"},   {PieceType::King, "King"},
  };

  for (const auto &[type, name] : pieces) {
    whiteTextures_[type] =
        LoadTexture(("assets/images/W_" + name + ".png").c_str());
    blackTextures_[type] =
        LoadTexture(("assets/images/B_" + name + ".png").c_str());
  }

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

inline void ChessView::drawPiece(PieceType type, ::Color color, float x,
                                 float y,
                                 float w, float h) {
  const auto &textures =
      (color == ::Color::White) ? whiteTextures_ : blackTextures_;
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

inline void ChessView::drawBoard(const Board &board,
                                 const std::optional<Position> &selectedSquare,
                                 const std::vector<Move> &legalMoves) {
  BeginDrawing();
  ClearBackground({0, 0, 0, 255});

  if (boardTexture_.id != 0) {
    const Rectangle src = {0.0f, 0.0f, static_cast<float>(boardTexture_.width),
                           static_cast<float>(boardTexture_.height)};
    const Rectangle dst = {0.0f, 0.0f, 512.0f, 512.0f};
    DrawTexturePro(boardTexture_, src, dst, {0.0f, 0.0f}, 0.0f,
                   {255, 255, 255, 255});
  }

  for (const auto &move : legalMoves) {
    DrawRectangle(move.to.col * 64, move.to.row * 64, 64, 64,
                  {0, 255, 0, 90});
  }

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      const Piece *piece = board.getPieceAt({row, col});
      if (piece) {
        drawPiece(piece->getType(), piece->getColor(),
                  static_cast<float>(col * 64), static_cast<float>(row * 64),
                  64.0f, 64.0f);
      }
    }
  }

  if (selectedSquare.has_value()) {
    DrawRectangleLinesEx(
        {static_cast<float>(selectedSquare->col * 64),
         static_cast<float>(selectedSquare->row * 64), 64.0f, 64.0f},
        3.0f, {255, 255, 0, 255});
  }

  EndDrawing();
}

#endif // CHESSVIEW_H
