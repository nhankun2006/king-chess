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
  std::map<PieceType, Rectangle> whiteSourceRects_;
  std::map<PieceType, Rectangle> blackSourceRects_;
  std::map<PieceType, float> whiteCenterOffsetX_;
  std::map<PieceType, float> blackCenterOffsetX_;
  std::string lastAssetError_;

  Rectangle getBoardGridRect() const;

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
  const std::string &getLastAssetError() const { return lastAssetError_; }
  std::optional<Position> screenToBoardSquare(float x, float y) const;
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
  lastAssetError_.clear();
  whiteSourceRects_.clear();
  blackSourceRects_.clear();
  whiteCenterOffsetX_.clear();
  blackCenterOffsetX_.clear();

  const char *appDir = GetApplicationDirectory();
  const std::vector<std::string> assetRoots = {
      "assets/images/", "../assets/images/", "../../assets/images/",
      (appDir ? std::string(appDir) : std::string()) + "assets/images/",
      (appDir ? std::string(appDir) : std::string()) + "../assets/images/",
      (appDir ? std::string(appDir) : std::string()) + "../../assets/images/"};

  auto loadTextureWithFallback = [&](const std::string &fileName,
                                     Texture2D &outTexture,
                                     std::string *loadedPath = nullptr) -> bool {
    for (const auto &root : assetRoots) {
      if (root.empty()) {
        continue;
      }

      const std::string path = root + fileName;
      if (!FileExists(path.c_str())) {
        continue;
      }

      outTexture = LoadTexture(path.c_str());
      if (outTexture.id != 0) {
        if (loadedPath != nullptr) {
          *loadedPath = path;
        }
        return true;
      }
    }
    return false;
  };

  struct PieceSourceMetrics {
    Rectangle rect;
    float centerOffsetX;
  };

  auto computePieceSourceMetrics = [&](const std::string &filePath,
                                       const Texture2D &texture)
      -> PieceSourceMetrics {
    const Rectangle fullTextureRect = {
        0.0f, 0.0f, static_cast<float>(texture.width),
        static_cast<float>(texture.height)};
    const PieceSourceMetrics fullTextureMetrics = {fullTextureRect, 0.0f};

    Image image = LoadImage(filePath.c_str());
    if (image.data == nullptr || image.width <= 0 || image.height <= 0) {
      return fullTextureMetrics;
    }

    auto *pixels = LoadImageColors(image);
    if (pixels == nullptr) {
      UnloadImage(image);
      return fullTextureMetrics;
    }

    int minX = image.width;
    int minY = image.height;
    int maxX = -1;
    int maxY = -1;

    for (int y = 0; y < image.height; ++y) {
      for (int x = 0; x < image.width; ++x) {
        const auto pixel = pixels[y * image.width + x];
        if (pixel.a > 0) {
          if (x < minX) {
            minX = x;
          }
          if (y < minY) {
            minY = y;
          }
          if (x > maxX) {
            maxX = x;
          }
          if (y > maxY) {
            maxY = y;
          }
        }
      }
    }

    if (maxX < minX || maxY < minY) {
      UnloadImageColors(pixels);
      UnloadImage(image);
      return fullTextureMetrics;
    }

    const Rectangle opaqueRect = {
        static_cast<float>(minX), static_cast<float>(minY),
        static_cast<float>(maxX - minX + 1),
        static_cast<float>(maxY - minY + 1)};

    const int opaqueHeight = maxY - minY + 1;
    const int baseBandHeight = (opaqueHeight / 5 > 0) ? (opaqueHeight / 5) : 1;
    const int baseYStart = maxY - baseBandHeight + 1;

    double baseWeightedSumX = 0.0;
    double baseTotalWeight = 0.0;
    for (int y = baseYStart; y <= maxY; ++y) {
      for (int x = minX; x <= maxX; ++x) {
        const auto pixel = pixels[y * image.width + x];
        if (pixel.a > 0) {
          const double alphaWeight = static_cast<double>(pixel.a);
          baseWeightedSumX += static_cast<double>(x) * alphaWeight;
          baseTotalWeight += alphaWeight;
        }
      }
    }

    const float boundingCenterX =
        (static_cast<float>(minX) + static_cast<float>(maxX)) * 0.5f;
    const float baseCenterX =
        (baseTotalWeight > 0.0)
            ? static_cast<float>(baseWeightedSumX / baseTotalWeight)
            : boundingCenterX;

    UnloadImageColors(pixels);
    UnloadImage(image);

    return {opaqueRect, boundingCenterX - baseCenterX};
  };

  if (!loadTextureWithFallback("board1.png", boardTexture_)) {
    return false;
  }

  static const std::pair<PieceType, std::string> pieces[] = {
      {PieceType::Pawn, "Pawn"},     {PieceType::Knight, "Knight"},
      {PieceType::Bishop, "Bishop"}, {PieceType::Rook, "Rook"},
      {PieceType::Queen, "Queen"},   {PieceType::King, "King"},
  };

  for (const auto &[type, name] : pieces) {
    std::string whiteTexturePath;
    if (!loadTextureWithFallback("W_" + name + ".png", whiteTextures_[type],
                                 &whiteTexturePath)) {
      return false;
    }

    std::string blackTexturePath;
    if (!loadTextureWithFallback("B_" + name + ".png", blackTextures_[type],
                                 &blackTexturePath)) {
      return false;
    }

    const PieceSourceMetrics whiteMetrics =
      computePieceSourceMetrics(whiteTexturePath, whiteTextures_[type]);
    const PieceSourceMetrics blackMetrics =
      computePieceSourceMetrics(blackTexturePath, blackTextures_[type]);

    whiteSourceRects_[type] = whiteMetrics.rect;
    blackSourceRects_[type] = blackMetrics.rect;
    whiteCenterOffsetX_[type] = whiteMetrics.centerOffsetX;
    blackCenterOffsetX_[type] = blackMetrics.centerOffsetX;
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

inline Rectangle ChessView::getBoardGridRect() const {
  constexpr float kBoardDstSize = 512.0f;
  constexpr float kBoardSourceSize = 142.0f;
  constexpr float kGridLeftPx = 7.0f;
  constexpr float kGridTopPx = 7.0f;
  constexpr float kGridRightPx = 7.0f;
  constexpr float kGridBottomPx = 8.0f;

  const float gridWidthPx = kBoardSourceSize - kGridLeftPx - kGridRightPx;
  const float gridHeightPx = kBoardSourceSize - kGridTopPx - kGridBottomPx;

  const float gridLeft = kBoardDstSize * (kGridLeftPx / kBoardSourceSize);
  const float gridTop = kBoardDstSize * (kGridTopPx / kBoardSourceSize);
  const float gridWidth = kBoardDstSize * (gridWidthPx / kBoardSourceSize);
  const float gridHeight = kBoardDstSize * (gridHeightPx / kBoardSourceSize);

  return {gridLeft, gridTop, gridWidth, gridHeight};
}

inline std::optional<Position> ChessView::screenToBoardSquare(float x,
                                                              float y) const {
  const Rectangle grid = getBoardGridRect();
  if (x < grid.x || y < grid.y || x >= grid.x + grid.width ||
      y >= grid.y + grid.height) {
    return std::nullopt;
  }

  const float cellW = grid.width / 8.0f;
  const float cellH = grid.height / 8.0f;
  const int col = static_cast<int>((x - grid.x) / cellW);
  const int row = static_cast<int>((y - grid.y) / cellH);

  return Position{row, col};
}

inline void ChessView::drawPiece(PieceType type, ::Color color, float x,
                                 float y,
                                 float w, float h) {
  const auto &textures =
      (color == ::Color::White) ? whiteTextures_ : blackTextures_;
  const auto &sourceRects =
      (color == ::Color::White) ? whiteSourceRects_ : blackSourceRects_;
  const auto &centerOffsets =
      (color == ::Color::White) ? whiteCenterOffsetX_ : blackCenterOffsetX_;
  const auto it = textures.find(type);
  if (it == textures.end() || it->second.id == 0) {
    return;
  }

  const Texture2D &texture = it->second;
  Rectangle src = {0.0f, 0.0f, static_cast<float>(texture.width),
                   static_cast<float>(texture.height)};
  const auto srcIt = sourceRects.find(type);
  if (srcIt != sourceRects.end() && srcIt->second.width > 0.0f &&
      srcIt->second.height > 0.0f) {
    src = srcIt->second;
  }

  const float scale = (w / src.width < h / src.height) ? (w / src.width)
                                                        : (h / src.height);
  constexpr float kPieceSizeFactor = 0.70f;
  const float dstWidth = src.width * scale * kPieceSizeFactor;
  const float dstHeight = src.height * scale * kPieceSizeFactor;
  constexpr float kPieceOffsetX = 0.0f;
  constexpr float kPieceBottomPadding = 3.0f;
  float autoCenterOffsetX = 0.0f;
  const auto centerOffsetIt = centerOffsets.find(type);
  if (centerOffsetIt != centerOffsets.end()) {
    autoCenterOffsetX = centerOffsetIt->second * scale * kPieceSizeFactor;
  }

  const Rectangle dst = {x + (w - dstWidth) * 0.5f + kPieceOffsetX +
                             autoCenterOffsetX,
                         y + h - dstHeight - kPieceBottomPadding,
                         dstWidth, dstHeight};
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

  const Rectangle grid = getBoardGridRect();
  const float cellW = grid.width / 8.0f;
  const float cellH = grid.height / 8.0f;

  for (const auto &move : legalMoves) {
    DrawRectangleRec(
        {grid.x + static_cast<float>(move.to.col) * cellW,
         grid.y + static_cast<float>(move.to.row) * cellH, cellW, cellH},
        {0, 255, 0, 90});
  }

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      const Piece *piece = board.getPieceAt({row, col});
      if (piece) {
        drawPiece(piece->getType(), piece->getColor(),
                  grid.x + static_cast<float>(col) * cellW,
                  grid.y + static_cast<float>(row) * cellH, cellW, cellH);
      }
    }
  }

  if (selectedSquare.has_value()) {
    DrawRectangleLinesEx(
        {grid.x + static_cast<float>(selectedSquare->col) * cellW,
         grid.y + static_cast<float>(selectedSquare->row) * cellH, cellW,
         cellH},
        3.0f, {255, 255, 0, 255});
  }

  EndDrawing();
}

#endif // CHESSVIEW_H
