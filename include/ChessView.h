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
  Texture2D settingIconTexture_{};
  Texture2D rotateIconTexture_{};
  Texture2D restartIconTexture_{};
  std::map<PieceType, Texture2D> whiteTextures_;
  std::map<PieceType, Texture2D> blackTextures_;
  std::map<PieceType, Rectangle> whiteSourceRects_;
  std::map<PieceType, Rectangle> blackSourceRects_;
  std::map<PieceType, float> whiteCenterOffsetX_;
  std::map<PieceType, float> blackCenterOffsetX_;
  bool isBoardFlipped_ = false;
  std::string lastAssetError_;

  Rectangle getBoardRenderRect() const;
  Rectangle getRightPanelRect() const;
  float getUiScale() const;
  Rectangle getSettingsButtonRect() const;
  Rectangle getBoardGridRect() const;
  Rectangle getRotateButtonRect() const;
  Rectangle getRestartButtonRect() const;
  Rectangle getRestartConfirmDialogRect() const;
  Rectangle getRestartConfirmYesButtonRect() const;
  Rectangle getRestartConfirmNoButtonRect() const;
  Rectangle getWindowSizeDialogRect() const;
  Rectangle getWindowSizeOptionRect(int index) const;
  Rectangle getWindowSizeCloseButtonRect() const;

public:
  ChessView() = default;

  ~ChessView() {
    if (boardTexture_.id != 0) {
      UnloadTexture(boardTexture_);
    }
    if (settingIconTexture_.id != 0) {
      UnloadTexture(settingIconTexture_);
    }
    if (rotateIconTexture_.id != 0) {
      UnloadTexture(rotateIconTexture_);
    }
    if (restartIconTexture_.id != 0) {
      UnloadTexture(restartIconTexture_);
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
  void toggleBoardOrientation() { isBoardFlipped_ = !isBoardFlipped_; }
  bool isSettingsButtonClicked(float x, float y) const;
  bool isRotateButtonClicked(float x, float y) const;
  bool isRestartButtonClicked(float x, float y) const;
  bool isRestartConfirmYesClicked(float x, float y) const;
  bool isRestartConfirmNoClicked(float x, float y) const;
  std::optional<int> getWindowSizeOptionClicked(float x, float y) const;
  bool isWindowSizeDialogCloseClicked(float x, float y) const;
  std::optional<Position> screenToBoardSquare(float x, float y) const;
  void drawPiece(PieceType type, ::Color color, float x, float y, float w,
                 float h);
  void drawBoard(const Board &board,
                 const std::optional<Position> &selectedSquare = std::nullopt,
                 const std::vector<Move> &legalMoves = {},
                 bool showRestartConfirm = false,
                 bool showWindowSizeDialog = false);

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
  if (settingIconTexture_.id != 0) {
    UnloadTexture(settingIconTexture_);
    settingIconTexture_ = {};
  }
  if (rotateIconTexture_.id != 0) {
    UnloadTexture(rotateIconTexture_);
    rotateIconTexture_ = {};
  }
  if (restartIconTexture_.id != 0) {
    UnloadTexture(restartIconTexture_);
    restartIconTexture_ = {};
  }
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

  loadTextureWithFallback("setting_feature.png", settingIconTexture_);
  loadTextureWithFallback("rotate_feature.png", rotateIconTexture_);
  loadTextureWithFallback("restart_feature.png", restartIconTexture_);

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
  const Rectangle boardRect = getBoardRenderRect();
  const float boardDstSize = boardRect.width;
  constexpr float kBoardSourceSize = 142.0f;
  constexpr float kGridLeftPx = 7.0f;
  constexpr float kGridTopPx = 7.0f;
  constexpr float kGridRightPx = 7.0f;
  constexpr float kGridBottomPx = 8.0f;

  const float gridWidthPx = kBoardSourceSize - kGridLeftPx - kGridRightPx;
  const float gridHeightPx = kBoardSourceSize - kGridTopPx - kGridBottomPx;

  const float gridLeft = boardRect.x + boardDstSize * (kGridLeftPx / kBoardSourceSize);
  const float gridTop = boardRect.y + boardDstSize * (kGridTopPx / kBoardSourceSize);
  const float gridWidth = boardDstSize * (gridWidthPx / kBoardSourceSize);
  const float gridHeight = boardDstSize * (gridHeightPx / kBoardSourceSize);

  return {gridLeft, gridTop, gridWidth, gridHeight};
}

inline Rectangle ChessView::getBoardRenderRect() const {
  const float screenWidth = static_cast<float>(GetScreenWidth());
  const float screenHeight = static_cast<float>(GetScreenHeight());

  constexpr float kMinPanelWidth = 180.0f;
  float maxBoardWidth = screenWidth - kMinPanelWidth;
  if (maxBoardWidth < 240.0f) {
    maxBoardWidth = screenWidth;
  }

  float boardSize = (screenHeight < maxBoardWidth) ? screenHeight : maxBoardWidth;
  if (boardSize < 240.0f) {
    boardSize = (screenHeight < screenWidth) ? screenHeight : screenWidth;
  }

  return {0.0f, 0.0f, boardSize, boardSize};
}

inline Rectangle ChessView::getRightPanelRect() const {
  const Rectangle boardRect = getBoardRenderRect();
  const float panelX = boardRect.x + boardRect.width;
  float panelWidth = static_cast<float>(GetScreenWidth()) - panelX;
  if (panelWidth < 0.0f) {
    panelWidth = 0.0f;
  }

  return {panelX, 0.0f, panelWidth, static_cast<float>(GetScreenHeight())};
}

inline float ChessView::getUiScale() const {
  const float base = getBoardRenderRect().width / 512.0f;
  if (base < 0.85f) {
    return 0.85f;
  }
  if (base > 1.35f) {
    return 1.35f;
  }
  return base;
}

inline Rectangle ChessView::getSettingsButtonRect() const {
  const Rectangle panel = getRightPanelRect();
  if (panel.width <= 0.0f) {
    return {0.0f, 0.0f, 0.0f, 0.0f};
  }

  const float uiScale = getUiScale();
  const float buttonSize = 44.0f * uiScale;
  const float buttonGap = 12.0f * uiScale;
  const float totalWidth = buttonSize * 3.0f + buttonGap * 2.0f;
  const float startX = panel.x + (panel.width - totalWidth) * 0.5f;
  const float startY = 236.0f * (getBoardRenderRect().height / 512.0f);
  return {startX, startY, buttonSize, buttonSize};
}

inline Rectangle ChessView::getRotateButtonRect() const {
  const Rectangle settingsButton = getSettingsButtonRect();
  return {settingsButton.x + settingsButton.width + 12.0f * getUiScale(),
          settingsButton.y, settingsButton.width, settingsButton.height};
}

inline Rectangle ChessView::getRestartButtonRect() const {
  const Rectangle rotateButton = getRotateButtonRect();
  return {rotateButton.x + rotateButton.width + 12.0f * getUiScale(),
          rotateButton.y,
          rotateButton.width, rotateButton.height};
}

inline Rectangle ChessView::getRestartConfirmDialogRect() const {
  const float uiScale = getUiScale();
  float kDialogWidth = 320.0f * uiScale;
  const float kDialogHeight = 150.0f * uiScale;
  const float maxWidth = static_cast<float>(GetScreenWidth()) - 40.0f;
  if (kDialogWidth > maxWidth) {
    kDialogWidth = maxWidth;
  }
  const float x = (static_cast<float>(GetScreenWidth()) - kDialogWidth) * 0.5f;
  const float y = (static_cast<float>(GetScreenHeight()) - kDialogHeight) * 0.5f;
  return {x, y, kDialogWidth, kDialogHeight};
}

inline Rectangle ChessView::getRestartConfirmYesButtonRect() const {
  const Rectangle dialog = getRestartConfirmDialogRect();
  return {dialog.x + 34.0f, dialog.y + dialog.height - 52.0f, 110.0f, 34.0f};
}

inline Rectangle ChessView::getRestartConfirmNoButtonRect() const {
  const Rectangle dialog = getRestartConfirmDialogRect();
  return {dialog.x + dialog.width - 34.0f - 110.0f,
          dialog.y + dialog.height - 52.0f, 110.0f, 34.0f};
}

inline Rectangle ChessView::getWindowSizeDialogRect() const {
  const float uiScale = getUiScale();
  float dialogWidth = 340.0f * uiScale;
  const float dialogHeight = 220.0f * uiScale;
  const float maxWidth = static_cast<float>(GetScreenWidth()) - 40.0f;
  if (dialogWidth > maxWidth) {
    dialogWidth = maxWidth;
  }

  const float x = (static_cast<float>(GetScreenWidth()) - dialogWidth) * 0.5f;
  const float y = (static_cast<float>(GetScreenHeight()) - dialogHeight) * 0.5f;
  return {x, y, dialogWidth, dialogHeight};
}

inline Rectangle ChessView::getWindowSizeOptionRect(int index) const {
  const Rectangle dialog = getWindowSizeDialogRect();
  const float uiScale = getUiScale();
  const float optionHeight = 40.0f * uiScale;
  const float gap = 10.0f * uiScale;
  const float startY = dialog.y + 54.0f * uiScale;
  return {dialog.x + 24.0f * uiScale,
          startY + static_cast<float>(index) * (optionHeight + gap),
          dialog.width - 48.0f * uiScale, optionHeight};
}

inline Rectangle ChessView::getWindowSizeCloseButtonRect() const {
  const Rectangle dialog = getWindowSizeDialogRect();
  const float uiScale = getUiScale();
  const float buttonSize = 28.0f * uiScale;
  return {dialog.x + dialog.width - buttonSize - 10.0f * uiScale,
          dialog.y + 10.0f * uiScale, buttonSize, buttonSize};
}

inline bool ChessView::isSettingsButtonClicked(float x, float y) const {
  const Rectangle settingsButton = getSettingsButtonRect();
  if (settingsButton.width <= 0.0f || settingsButton.height <= 0.0f) {
    return false;
  }

  return x >= settingsButton.x && x <= settingsButton.x + settingsButton.width &&
         y >= settingsButton.y && y <= settingsButton.y + settingsButton.height;
}

inline bool ChessView::isRotateButtonClicked(float x, float y) const {
  const Rectangle rotateButton = getRotateButtonRect();
  if (rotateButton.width <= 0.0f || rotateButton.height <= 0.0f) {
    return false;
  }

  return x >= rotateButton.x && x <= rotateButton.x + rotateButton.width &&
         y >= rotateButton.y && y <= rotateButton.y + rotateButton.height;
}

inline bool ChessView::isRestartButtonClicked(float x, float y) const {
  const Rectangle restartButton = getRestartButtonRect();
  if (restartButton.width <= 0.0f || restartButton.height <= 0.0f) {
    return false;
  }

  return x >= restartButton.x && x <= restartButton.x + restartButton.width &&
         y >= restartButton.y && y <= restartButton.y + restartButton.height;
}

inline std::optional<int> ChessView::getWindowSizeOptionClicked(float x,
                                                                float y) const {
  for (int index = 0; index < 3; ++index) {
    const Rectangle option = getWindowSizeOptionRect(index);
    if (x >= option.x && x <= option.x + option.width && y >= option.y &&
        y <= option.y + option.height) {
      return index;
    }
  }

  return std::nullopt;
}

inline bool ChessView::isWindowSizeDialogCloseClicked(float x, float y) const {
  const Rectangle closeButton = getWindowSizeCloseButtonRect();
  return x >= closeButton.x && x <= closeButton.x + closeButton.width &&
         y >= closeButton.y && y <= closeButton.y + closeButton.height;
}

inline bool ChessView::isRestartConfirmYesClicked(float x, float y) const {
  const Rectangle yesButton = getRestartConfirmYesButtonRect();
  return x >= yesButton.x && x <= yesButton.x + yesButton.width &&
         y >= yesButton.y && y <= yesButton.y + yesButton.height;
}

inline bool ChessView::isRestartConfirmNoClicked(float x, float y) const {
  const Rectangle noButton = getRestartConfirmNoButtonRect();
  return x >= noButton.x && x <= noButton.x + noButton.width &&
         y >= noButton.y && y <= noButton.y + noButton.height;
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
  const int displayCol = static_cast<int>((x - grid.x) / cellW);
  const int displayRow = static_cast<int>((y - grid.y) / cellH);

  const int col = isBoardFlipped_ ? (7 - displayCol) : displayCol;
  const int row = isBoardFlipped_ ? (7 - displayRow) : displayRow;

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
                                 const std::vector<Move> &legalMoves,
                                 bool showRestartConfirm,
                                 bool showWindowSizeDialog) {
  BeginDrawing();
  ClearBackground({0, 0, 0, 255});

  if (boardTexture_.id != 0) {
    Rectangle src = {0.0f, 0.0f, static_cast<float>(boardTexture_.width),
                     static_cast<float>(boardTexture_.height)};
    if (isBoardFlipped_) {
      src = {static_cast<float>(boardTexture_.width),
             static_cast<float>(boardTexture_.height),
             -static_cast<float>(boardTexture_.width),
             -static_cast<float>(boardTexture_.height)};
    }
    const Rectangle dst = getBoardRenderRect();
    DrawTexturePro(boardTexture_, src, dst, {0.0f, 0.0f}, 0.0f,
                   {255, 255, 255, 255});
  }

  const Rectangle grid = getBoardGridRect();
  const float cellW = grid.width / 8.0f;
  const float cellH = grid.height / 8.0f;

  for (const auto &move : legalMoves) {
    const int displayCol = isBoardFlipped_ ? (7 - move.to.col) : move.to.col;
    const int displayRow = isBoardFlipped_ ? (7 - move.to.row) : move.to.row;
    DrawRectangleRec(
        {grid.x + static_cast<float>(displayCol) * cellW,
         grid.y + static_cast<float>(displayRow) * cellH, cellW, cellH},
        {0, 255, 0, 90});
  }

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      const Piece *piece = board.getPieceAt({row, col});
      if (piece) {
        const int displayCol = isBoardFlipped_ ? (7 - col) : col;
        const int displayRow = isBoardFlipped_ ? (7 - row) : row;
        drawPiece(piece->getType(), piece->getColor(),
                  grid.x + static_cast<float>(displayCol) * cellW,
                  grid.y + static_cast<float>(displayRow) * cellH, cellW,
                  cellH);
      }
    }
  }

  if (selectedSquare.has_value()) {
    const int displayCol =
        isBoardFlipped_ ? (7 - selectedSquare->col) : selectedSquare->col;
    const int displayRow =
        isBoardFlipped_ ? (7 - selectedSquare->row) : selectedSquare->row;
    DrawRectangleLinesEx(
        {grid.x + static_cast<float>(displayCol) * cellW,
         grid.y + static_cast<float>(displayRow) * cellH, cellW, cellH},
        3.0f, {255, 255, 0, 255});
  }

  auto getInitialCount = [](PieceType type) -> int {
    switch (type) {
    case PieceType::Pawn:
      return 8;
    case PieceType::Knight:
    case PieceType::Bishop:
    case PieceType::Rook:
      return 2;
    case PieceType::Queen:
    case PieceType::King:
      return 1;
    default:
      return 0;
    }
  };

  std::map<PieceType, int> whiteRemaining;
  std::map<PieceType, int> blackRemaining;

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      const Piece *piece = board.getPieceAt({row, col});
      if (piece == nullptr) {
        continue;
      }

      const PieceType type = piece->getType();
      if (piece->getColor() == ::Color::White) {
        whiteRemaining[type] += 1;
      } else {
        blackRemaining[type] += 1;
      }
    }
  }

  std::map<PieceType, int> capturedBlackPieces;
  std::map<PieceType, int> capturedWhitePieces;
  const std::vector<PieceType> pieceOrder = {
      PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight,
      PieceType::Pawn};

  for (const PieceType type : pieceOrder) {
    int blackCaptured = getInitialCount(type) - blackRemaining[type];
    if (blackCaptured < 0) {
      blackCaptured = 0;
    }
    int whiteCaptured = getInitialCount(type) - whiteRemaining[type];
    if (whiteCaptured < 0) {
      whiteCaptured = 0;
    }
    capturedBlackPieces[type] = blackCaptured;
    capturedWhitePieces[type] = whiteCaptured;
  }

  auto drawCapturedSection = [&](int sectionX, int sectionY, int sectionWidth,
                                 const char *title, ::Color capturedColor,
                                 const std::map<PieceType, int> &captured)
      -> int {
    const float uiScale = getUiScale();
    const int titleFontSize = static_cast<int>(20.0f * uiScale);
    DrawText(title, sectionX + static_cast<int>(12.0f * uiScale),
             sectionY + static_cast<int>(8.0f * uiScale), titleFontSize,
             {235, 235, 235, 255});

    const float iconW = 24.0f * uiScale;
    const float iconH = 24.0f * uiScale;
    const float gap = 4.0f * uiScale;
    const int maxPerRow =
        (sectionWidth - static_cast<int>(20.0f * uiScale)) /
        static_cast<int>(iconW + gap);
    const int safeMaxPerRow = (maxPerRow > 0) ? maxPerRow : 1;
    float iconX = static_cast<float>(sectionX) + 10.0f * uiScale;
    float iconY = static_cast<float>(sectionY) + 36.0f * uiScale;
    int drawn = 0;

    for (const PieceType type : pieceOrder) {
      const int count = captured.at(type);
      for (int index = 0; index < count; ++index) {
        drawPiece(type, capturedColor, iconX, iconY, iconW, iconH);
        drawn += 1;
        if (drawn % safeMaxPerRow == 0) {
          iconX = static_cast<float>(sectionX) + 10.0f * uiScale;
          iconY += iconH + gap;
        } else {
          iconX += iconW + gap;
        }
      }
    }

    if (drawn == 0) {
      DrawText("-", sectionX + static_cast<int>(14.0f * uiScale),
               sectionY + static_cast<int>(38.0f * uiScale),
               static_cast<int>(24.0f * uiScale), {180, 180, 180, 255});
      return static_cast<int>(68.0f * uiScale);
    }

    const int rows = (drawn + safeMaxPerRow - 1) / safeMaxPerRow;
    return static_cast<int>(40.0f * uiScale) +
           rows * static_cast<int>(iconH + gap);
  };

  auto getCapturedSectionHeight = [&](int sectionWidth,
                                      const std::map<PieceType, int> &captured)
      -> int {
    const float uiScale = getUiScale();
    const float iconW = 24.0f * uiScale;
    const float iconH = 24.0f * uiScale;
    const float gap = 4.0f * uiScale;
    const int maxPerRow =
        (sectionWidth - static_cast<int>(20.0f * uiScale)) /
        static_cast<int>(iconW + gap);
    const int safeMaxPerRow = (maxPerRow > 0) ? maxPerRow : 1;

    int drawn = 0;
    for (const PieceType type : pieceOrder) {
      drawn += captured.at(type);
    }

    if (drawn == 0) {
      return static_cast<int>(68.0f * uiScale);
    }

    const int rows = (drawn + safeMaxPerRow - 1) / safeMaxPerRow;
    return static_cast<int>(40.0f * uiScale) +
           rows * static_cast<int>(iconH + gap);
  };

  const Rectangle rightPanel = getRightPanelRect();
  const int rightPanelX = static_cast<int>(rightPanel.x);
  const int rightPanelWidth = static_cast<int>(rightPanel.width);
  const int rightPanelHeight = static_cast<int>(rightPanel.height);

  if (rightPanelWidth > 0) {
    DrawRectangle(rightPanelX, 0, rightPanelWidth, rightPanelHeight,
                  {26, 30, 40, 255});
    DrawLine(rightPanelX, 0, rightPanelX, rightPanelHeight,
             {70, 80, 100, 255});

    const Vector2 mousePos = GetMousePosition();
    const Rectangle settingsButton = getSettingsButtonRect();
    const Rectangle rotateButton = getRotateButtonRect();
    const Rectangle restartButton = getRestartButtonRect();
    const bool settingsHovered = CheckCollisionPointRec(mousePos, settingsButton);
    const bool rotateHovered = CheckCollisionPointRec(mousePos, rotateButton);
    const bool restartHovered =
        CheckCollisionPointRec(mousePos, restartButton);

    auto drawRoundedIconButton = [&](Rectangle buttonRect, bool hovered) {
      const float uiScale = getUiScale();
      if (hovered) {
        const float boost = 1.5f * uiScale;
        buttonRect.x -= boost;
        buttonRect.y -= boost;
        buttonRect.width += boost * 2.0f;
        buttonRect.height += boost * 2.0f;
      }

      DrawRectangleRounded(buttonRect, 0.35f, 10,
               hovered ? GetColor(0x485670FF)
                 : GetColor(0x303A4EFF));
      DrawRectangleRoundedLinesEx(buttonRect, 0.35f, 10, 2.0f,
                hovered ? GetColor(0x8498BEFF)
                  : GetColor(0x5A6982FF));
    };

    drawRoundedIconButton(settingsButton, settingsHovered);
    drawRoundedIconButton(rotateButton, rotateHovered);
    drawRoundedIconButton(restartButton, restartHovered);
    const float uiScale = getUiScale();
    const float buttonIconSize = 24.0f * uiScale;

    if (settingIconTexture_.id != 0) {
      const Rectangle iconSrc = {0.0f, 0.0f,
                   static_cast<float>(settingIconTexture_.width),
                   static_cast<float>(settingIconTexture_.height)};
      const Rectangle iconDst = {
        settingsButton.x + (settingsButton.width - buttonIconSize) * 0.5f,
        settingsButton.y + (settingsButton.height - buttonIconSize) * 0.5f,
        buttonIconSize, buttonIconSize};
      DrawTexturePro(settingIconTexture_, iconSrc, iconDst, {0.0f, 0.0f},
             0.0f, {255, 255, 255, 255});
    } else {
      DrawCircleV(
        {settingsButton.x + settingsButton.width * 0.5f,
         settingsButton.y + settingsButton.height * 0.5f},
        9.0f * uiScale, {235, 235, 235, 255});
      DrawCircleV(
        {settingsButton.x + settingsButton.width * 0.5f,
         settingsButton.y + settingsButton.height * 0.5f},
        4.5f * uiScale, {48, 58, 78, 255});
    }

    if (rotateIconTexture_.id != 0) {
      const Rectangle iconSrc = {0.0f, 0.0f,
                                 static_cast<float>(rotateIconTexture_.width),
                                 static_cast<float>(rotateIconTexture_.height)};
      const Rectangle iconDst = {rotateButton.x +
                       (rotateButton.width - buttonIconSize) * 0.5f,
                                 rotateButton.y +
                       (rotateButton.height - buttonIconSize) * 0.5f,
                     buttonIconSize, buttonIconSize};
      DrawTexturePro(rotateIconTexture_, iconSrc, iconDst, {0.0f, 0.0f}, 0.0f,
                     {255, 255, 255, 255});
    } else {
        DrawText("R", static_cast<int>(rotateButton.x + 14.0f * uiScale),
             static_cast<int>(rotateButton.y + 9.0f * uiScale),
             static_cast<int>(24.0f * uiScale),
               {235, 235, 235, 255});
    }

    if (restartIconTexture_.id != 0) {
      const Rectangle iconSrc = {0.0f, 0.0f,
                                 static_cast<float>(restartIconTexture_.width),
                                 static_cast<float>(restartIconTexture_.height)};
      const Rectangle iconDst = {restartButton.x +
                                     (restartButton.width - buttonIconSize) * 0.5f,
                                 restartButton.y +
                                     (restartButton.height - buttonIconSize) * 0.5f,
                                 buttonIconSize, buttonIconSize};
      DrawTexturePro(restartIconTexture_, iconSrc, iconDst, {0.0f, 0.0f},
                     0.0f, {255, 255, 255, 255});
    } else {
      const float cx = restartButton.x + restartButton.width * 0.5f;
      const float cy = restartButton.y + restartButton.height * 0.5f;
      DrawRing({cx, cy}, 8.0f * uiScale, 12.0f * uiScale, 35.0f, 320.0f, 24,
               {235, 235, 235, 255});
      DrawTriangle({cx + 9.0f * uiScale, cy - 12.0f * uiScale},
                   {cx + 15.0f * uiScale, cy - 6.0f * uiScale},
                   {cx + 6.0f * uiScale, cy - 4.0f * uiScale},
                   {235, 235, 235, 255});
    }

    auto drawCenteredHoverText = [&](Rectangle buttonRect, const char *text) {
      const int kTooltipFontSize = static_cast<int>(14.0f * getUiScale());
      const int textWidth = MeasureText(text, kTooltipFontSize);
      const float buttonCenterX = buttonRect.x + buttonRect.width * 0.5f;
      const int textX = static_cast<int>(buttonCenterX - textWidth * 0.5f);
      const int textY =
          static_cast<int>(buttonRect.y - 20.0f * getUiScale());
      DrawText(text, textX, textY, kTooltipFontSize, {210, 220, 235, 255});
    };

    if (rotateHovered) {
      drawCenteredHoverText(rotateButton, "Rotate board");
    }
    if (restartHovered) {
      drawCenteredHoverText(restartButton, "Restart game");
    }
    if (settingsHovered) {
      drawCenteredHoverText(settingsButton, "Window size");
    }

    const int whiteSectionY = 8;
    const int whiteSectionBottom =
        whiteSectionY + drawCapturedSection(rightPanelX, whiteSectionY,
                                            rightPanelWidth, "White captured",
                                            ::Color::Black,
                                            capturedBlackPieces);

    const int blackSectionHeight =
        getCapturedSectionHeight(rightPanelWidth, capturedWhitePieces);
    int blackSectionY = rightPanelHeight - 8 - blackSectionHeight;
    if (blackSectionY < whiteSectionBottom + 12) {
      blackSectionY = whiteSectionBottom + 12;
    }

    drawCapturedSection(rightPanelX, blackSectionY, rightPanelWidth,
                        "Black captured", ::Color::White,
                        capturedWhitePieces);
  }

  if (showRestartConfirm) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                  {0, 0, 0, 120});

    const Rectangle dialog = getRestartConfirmDialogRect();
    const Rectangle yesButton = getRestartConfirmYesButtonRect();
    const Rectangle noButton = getRestartConfirmNoButtonRect();
    const Vector2 mousePos = GetMousePosition();

    const float uiScale = getUiScale();
    const int titleFontSize = static_cast<int>(24.0f * uiScale);
    const int bodyFontSize = static_cast<int>(16.0f * uiScale);
    const int actionFontSize = static_cast<int>(20.0f * uiScale);
    const char *titleText = "Restart game?";
    const char *bodyText = "Current match progress will be lost.";
    const char *yesText = "Yes";
    const char *noText = "No";

    const int titleTextWidth = MeasureText(titleText, titleFontSize);
    const int bodyTextWidth = MeasureText(bodyText, bodyFontSize);
    const int yesTextWidth = MeasureText(yesText, actionFontSize);
    const int noTextWidth = MeasureText(noText, actionFontSize);

    const int titleX = static_cast<int>(dialog.x +
                      (dialog.width - static_cast<float>(titleTextWidth)) *
                        0.5f);
    const int titleY = static_cast<int>(dialog.y + 22.0f * uiScale);
    const int bodyX = static_cast<int>(dialog.x +
                       (dialog.width - static_cast<float>(bodyTextWidth)) *
                         0.5f);
    const int bodyY = static_cast<int>(dialog.y + 58.0f * uiScale);

    DrawRectangleRounded(dialog, 0.16f, 12, {32, 38, 50, 255});
    DrawRectangleRoundedLinesEx(dialog, 0.16f, 12, 2.0f, {110, 124, 150, 255});
        DrawText(titleText, titleX, titleY, titleFontSize, {235, 235, 235, 255});
        DrawText(bodyText, bodyX, bodyY, bodyFontSize, {195, 205, 220, 255});

    const bool yesHovered = CheckCollisionPointRec(mousePos, yesButton);
    const bool noHovered = CheckCollisionPointRec(mousePos, noButton);

    DrawRectangleRounded(yesButton, 0.25f, 8,
                         yesHovered ? GetColor(0x3E8D5CFF)
                                    : GetColor(0x2F724AFF));
    DrawRectangleRoundedLinesEx(yesButton, 0.25f, 8, 2.0f,
                                yesHovered ? GetColor(0x6EC798FF)
                                           : GetColor(0x4A9A6BFF));
    DrawText(yesText,
         static_cast<int>(yesButton.x +
                  (yesButton.width - static_cast<float>(yesTextWidth)) * 0.5f),
         static_cast<int>(yesButton.y +
                  (yesButton.height - static_cast<float>(actionFontSize)) * 0.5f),
         actionFontSize, {240, 245, 240, 255});

    DrawRectangleRounded(noButton, 0.25f, 8,
                         noHovered ? GetColor(0x8A4545FF)
                                   : GetColor(0x703838FF));
    DrawRectangleRoundedLinesEx(noButton, 0.25f, 8, 2.0f,
                                noHovered ? GetColor(0xCF7C7CFF)
                                          : GetColor(0xA55D5DFF));
    DrawText(noText,
         static_cast<int>(noButton.x +
                  (noButton.width - static_cast<float>(noTextWidth)) * 0.5f),
         static_cast<int>(noButton.y +
                  (noButton.height - static_cast<float>(actionFontSize)) * 0.5f),
         actionFontSize, {245, 240, 240, 255});
  }

  if (showWindowSizeDialog) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                  {0, 0, 0, 120});

    const Rectangle dialog = getWindowSizeDialogRect();
    const Rectangle closeButton = getWindowSizeCloseButtonRect();
    const Vector2 mousePos = GetMousePosition();
    const float uiScale = getUiScale();

    const int titleFontSize = static_cast<int>(22.0f * uiScale);
    const int optionFontSize = static_cast<int>(18.0f * uiScale);
    const int closeFontSize = static_cast<int>(20.0f * uiScale);

    DrawRectangleRounded(dialog, 0.16f, 12, {32, 38, 50, 255});
    DrawRectangleRoundedLinesEx(dialog, 0.16f, 12, 2.0f, {110, 124, 150, 255});

    const char *titleText = "Window size";
    const int titleWidth = MeasureText(titleText, titleFontSize);
    DrawText(titleText,
             static_cast<int>(dialog.x +
                              (dialog.width - static_cast<float>(titleWidth)) *
                                  0.5f),
             static_cast<int>(dialog.y + 16.0f * uiScale), titleFontSize,
             {235, 235, 235, 255});

    const char *sizeLabels[3] = {"700 x 512", "900 x 650", "1100 x 780"};
    for (int index = 0; index < 3; ++index) {
      const Rectangle optionRect = getWindowSizeOptionRect(index);
      const bool hovered = CheckCollisionPointRec(mousePos, optionRect);

      DrawRectangleRounded(optionRect, 0.22f, 8,
                           hovered ? GetColor(0x485670FF)
                                   : GetColor(0x303A4EFF));
      DrawRectangleRoundedLinesEx(optionRect, 0.22f, 8, 2.0f,
                                  hovered ? GetColor(0x8498BEFF)
                                          : GetColor(0x5A6982FF));

      const int labelWidth = MeasureText(sizeLabels[index], optionFontSize);
      DrawText(sizeLabels[index],
               static_cast<int>(optionRect.x +
                                (optionRect.width - static_cast<float>(labelWidth)) *
                                    0.5f),
               static_cast<int>(optionRect.y +
                                (optionRect.height -
                                 static_cast<float>(optionFontSize)) *
                                    0.5f),
               optionFontSize, {235, 235, 235, 255});
    }

    const bool closeHovered = CheckCollisionPointRec(mousePos, closeButton);
    DrawRectangleRounded(closeButton, 0.25f, 8,
                         closeHovered ? GetColor(0x8A4545FF)
                                     : GetColor(0x703838FF));
    DrawRectangleRoundedLinesEx(closeButton, 0.25f, 8, 2.0f,
                                closeHovered ? GetColor(0xCF7C7CFF)
                                            : GetColor(0xA55D5DFF));
    const char *closeText = "X";
    const int closeWidth = MeasureText(closeText, closeFontSize);
    DrawText(closeText,
             static_cast<int>(closeButton.x +
                              (closeButton.width - static_cast<float>(closeWidth)) *
                                  0.5f),
             static_cast<int>(closeButton.y +
                              (closeButton.height -
                               static_cast<float>(closeFontSize)) *
                                  0.5f),
             closeFontSize, {245, 240, 240, 255});
  }

  EndDrawing();
}

#endif // CHESSVIEW_H
