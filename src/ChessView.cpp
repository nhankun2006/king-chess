#include "ChessView.h"

#include <cmath>

#include "UIConfig.h"

namespace {
constexpr PieceType kPromotionOptions[ui::Dialog::kPromotionOptionCount] = {
    PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight};
}

ChessView::~ChessView() {
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
  if (burningLoopTexture_.id != 0) {
    UnloadTexture(burningLoopTexture_);
  }
  if (burningLoop2Texture_.id != 0) {
    UnloadTexture(burningLoop2Texture_);
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

bool ChessView::LoadAssets() {
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
  if (burningLoopTexture_.id != 0) {
    UnloadTexture(burningLoopTexture_);
    burningLoopTexture_ = {};
  }
  if (burningLoop2Texture_.id != 0) {
    UnloadTexture(burningLoop2Texture_);
    burningLoop2Texture_ = {};
  }
  whiteSourceRects_.clear();
  blackSourceRects_.clear();
  whiteCenterOffsetX_.clear();
  blackCenterOffsetX_.clear();

  const char *appDir = GetApplicationDirectory();
  const std::vector<std::string> assetRoots = {
      "assets/images/",
      "../assets/images/",
      "../../assets/images/",
      (appDir ? std::string(appDir) : std::string()) + "assets/images/",
      (appDir ? std::string(appDir) : std::string()) + "../assets/images/",
      (appDir ? std::string(appDir) : std::string()) + "../../assets/images/"};

  auto loadTextureWithFallback =
      [&](const std::string &fileName, Texture2D &outTexture,
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

  auto computePieceSourceMetrics =
      [&](const std::string &filePath,
          const Texture2D &texture) -> PieceSourceMetrics {
    const Rectangle fullTextureRect = {0.0f, 0.0f,
                                       static_cast<float>(texture.width),
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

    const Rectangle opaqueRect = {static_cast<float>(minX),
                                  static_cast<float>(minY),
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
  loadTextureWithFallback("burning_loop_1.png", burningLoopTexture_);
  loadTextureWithFallback("burning_loop_2.png", burningLoop2Texture_);

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

Rectangle ChessView::getBoardGridRect() const {
  const Rectangle boardRect = getBoardRenderRect();
  const float boardDstSize = boardRect.width;
  const float gridWidthPx = ui::Board::kSourceSize - ui::Board::kGridLeftPx -
                            ui::Board::kGridRightPx;
  const float gridHeightPx = ui::Board::kSourceSize - ui::Board::kGridTopPx -
                             ui::Board::kGridBottomPx;

  const float gridLeft =
      boardRect.x + boardDstSize * (ui::Board::kGridLeftPx / ui::Board::kSourceSize);
  const float gridTop =
      boardRect.y + boardDstSize * (ui::Board::kGridTopPx / ui::Board::kSourceSize);
  const float gridWidth = boardDstSize * (gridWidthPx / ui::Board::kSourceSize);
  const float gridHeight = boardDstSize * (gridHeightPx / ui::Board::kSourceSize);

  return {gridLeft, gridTop, gridWidth, gridHeight};
}

Rectangle ChessView::getBoardRenderRect() const {
  const ui::AutoLayout::Metrics m =
      ui::AutoLayout::ComputeMetrics(GetScreenWidth(), GetScreenHeight());
  return m.boardRect;
}

Rectangle ChessView::getRightPanelRect() const {
  const ui::AutoLayout::Metrics m =
      ui::AutoLayout::ComputeMetrics(GetScreenWidth(), GetScreenHeight());
  return m.rightPanelRect;
}

float ChessView::getUiScale() const {
  const ui::AutoLayout::Metrics m =
      ui::AutoLayout::ComputeMetrics(GetScreenWidth(), GetScreenHeight());
  return m.uiScale;
}

Rectangle ChessView::getSettingsButtonRect() const {
  const Rectangle panel = getRightPanelRect();
  if (panel.width <= 0.0f) {
    return {0.0f, 0.0f, 0.0f, 0.0f};
  }

  const ui::AutoLayout::Metrics m =
      ui::AutoLayout::ComputeMetrics(GetScreenWidth(), GetScreenHeight());
  const float buttonSize = ui::AutoLayout::IconButtonSize(m);
  const float buttonGap = ui::AutoLayout::IconButtonGap(m);
  const float totalWidth = buttonSize * 3.0f + buttonGap * 2.0f;
  const float startX = panel.x + (panel.width - totalWidth) * 0.5f;
  const float startY = ui::AutoLayout::IconButtonStartY(m);
  return {startX, startY, buttonSize, buttonSize};
}

Rectangle ChessView::getRotateButtonRect() const {
  const Rectangle settingsButton = getSettingsButtonRect();
  const ui::AutoLayout::Metrics m =
      ui::AutoLayout::ComputeMetrics(GetScreenWidth(), GetScreenHeight());
  return {settingsButton.x + settingsButton.width + ui::AutoLayout::IconButtonGap(m),
          settingsButton.y, settingsButton.width, settingsButton.height};
}

Rectangle ChessView::getRestartButtonRect() const {
  const Rectangle rotateButton = getRotateButtonRect();
  const ui::AutoLayout::Metrics m =
      ui::AutoLayout::ComputeMetrics(GetScreenWidth(), GetScreenHeight());
  return {rotateButton.x + rotateButton.width + ui::AutoLayout::IconButtonGap(m),
           rotateButton.y, rotateButton.width, rotateButton.height};
}

Rectangle ChessView::getRestartConfirmDialogRect() const {
  const ui::AutoLayout::Metrics m =
      ui::AutoLayout::ComputeMetrics(GetScreenWidth(), GetScreenHeight());
  return ui::AutoLayout::RestartDialogRect(m);
}

Rectangle ChessView::getRestartConfirmYesButtonRect() const {
  const Rectangle dialog = getRestartConfirmDialogRect();
  constexpr float kInsetXRatio = 34.0f / 320.0f;
  constexpr float kBottomRatio = 52.0f / 150.0f;
  constexpr float kWidthRatio = 110.0f / 320.0f;
  constexpr float kHeightRatio = 34.0f / 150.0f;
  return {dialog.x + dialog.width * kInsetXRatio,
          dialog.y + dialog.height - dialog.height * kBottomRatio,
          dialog.width * kWidthRatio, dialog.height * kHeightRatio};
}

Rectangle ChessView::getRestartConfirmNoButtonRect() const {
  const Rectangle dialog = getRestartConfirmDialogRect();
  constexpr float kInsetXRatio = 34.0f / 320.0f;
  constexpr float kBottomRatio = 52.0f / 150.0f;
  constexpr float kWidthRatio = 110.0f / 320.0f;
  constexpr float kHeightRatio = 34.0f / 150.0f;
  const float buttonW = dialog.width * kWidthRatio;
  return {dialog.x + dialog.width - dialog.width * kInsetXRatio - buttonW,
          dialog.y + dialog.height - dialog.height * kBottomRatio, buttonW,
          dialog.height * kHeightRatio};
}

Rectangle ChessView::getWindowSizeDialogRect() const {
  const ui::AutoLayout::Metrics m =
      ui::AutoLayout::ComputeMetrics(GetScreenWidth(), GetScreenHeight());
  return ui::AutoLayout::WindowSizeDialogRect(m);
}

Rectangle ChessView::getWindowSizeOptionRect(int index) const {
  const Rectangle dialog = getWindowSizeDialogRect();
  constexpr float kHeightRatio = 40.0f / 270.0f;
  constexpr float kGapRatio = 10.0f / 270.0f;
  constexpr float kStartYRatio = 54.0f / 270.0f;
  constexpr float kPadXRatio = 24.0f / 340.0f;
  const float optionHeight = dialog.height * kHeightRatio;
  const float gap = dialog.height * kGapRatio;
  const float startY = dialog.y + dialog.height * kStartYRatio;
  const float padX = dialog.width * kPadXRatio;
  return {dialog.x + padX,
          startY + static_cast<float>(index) * (optionHeight + gap),
          dialog.width - 2.0f * padX, optionHeight};
}

Rectangle ChessView::getWindowSizeCloseButtonRect() const {
  const Rectangle dialog = getWindowSizeDialogRect();
  constexpr float kSizeRatio = 28.0f / 340.0f;
  constexpr float kMarginRatio = 10.0f / 340.0f;
  const float buttonSize = dialog.width * kSizeRatio;
  const float margin = dialog.width * kMarginRatio;
  return {dialog.x + dialog.width - buttonSize - margin,
          dialog.y + margin, buttonSize,
          buttonSize};
}

Rectangle ChessView::getPromotionDialogRect() const {
  const ui::AutoLayout::Metrics m =
      ui::AutoLayout::ComputeMetrics(GetScreenWidth(), GetScreenHeight());
  return ui::AutoLayout::PromotionDialogRect(m);
}

Rectangle ChessView::getPromotionOptionRect(int index) const {
  const Rectangle dialog = getPromotionDialogRect();
  constexpr float kGapRatio = 10.0f / 320.0f;
  constexpr float kTotalPadRatio = 36.0f / 320.0f;
  constexpr float kStartXRatio = 18.0f / 320.0f;
  constexpr float kYRatio = 52.0f / 130.0f;
  const float gap = dialog.width * kGapRatio;
  const float totalGap =
      gap * static_cast<float>(ui::Dialog::kPromotionOptionCount - 1);
  const float buttonSize =
      (dialog.width - dialog.width * kTotalPadRatio - totalGap) /
      static_cast<float>(ui::Dialog::kPromotionOptionCount);
  const float startX = dialog.x + dialog.width * kStartXRatio;
  const float y = dialog.y + dialog.height * kYRatio;
  return {startX + static_cast<float>(index) * (buttonSize + gap), y,
          buttonSize, buttonSize};
}

int ChessView::boardToDisplayIndex(int boardIndex) const {
  return isBoardFlipped_ ? (ui::Board::kMaxIndex - boardIndex) : boardIndex;
}

Rectangle ChessView::getDisplaySquareRect(int displayRow, int displayCol) const {
  const Rectangle grid = getBoardGridRect();
  const float cellW = grid.width / static_cast<float>(ui::Board::kSquaresPerSide);
  const float cellH =
      grid.height / static_cast<float>(ui::Board::kSquaresPerSide);
  return {grid.x + static_cast<float>(displayCol) * cellW,
          grid.y + static_cast<float>(displayRow) * cellH, cellW, cellH};
}

Rectangle ChessView::getBoardSquareRect(Position boardPos) const {
  return getDisplaySquareRect(boardToDisplayIndex(boardPos.row),
                              boardToDisplayIndex(boardPos.col));
}

float ChessView::clamp01(float value) {
  if (value < 0.0f) {
    return 0.0f;
  }
  if (value > 1.0f) {
    return 1.0f;
  }
  return value;
}

bool ChessView::isSettingsButtonClicked(float x, float y) const {
  const Rectangle settingsButton = getSettingsButtonRect();
  if (settingsButton.width <= 0.0f || settingsButton.height <= 0.0f) {
    return false;
  }

  return x >= settingsButton.x &&
         x <= settingsButton.x + settingsButton.width &&
         y >= settingsButton.y && y <= settingsButton.y + settingsButton.height;
}

bool ChessView::isRotateButtonClicked(float x, float y) const {
  const Rectangle rotateButton = getRotateButtonRect();
  if (rotateButton.width <= 0.0f || rotateButton.height <= 0.0f) {
    return false;
  }

  return x >= rotateButton.x && x <= rotateButton.x + rotateButton.width &&
         y >= rotateButton.y && y <= rotateButton.y + rotateButton.height;
}

bool ChessView::isRestartButtonClicked(float x, float y) const {
  const Rectangle restartButton = getRestartButtonRect();
  if (restartButton.width <= 0.0f || restartButton.height <= 0.0f) {
    return false;
  }

  return x >= restartButton.x && x <= restartButton.x + restartButton.width &&
         y >= restartButton.y && y <= restartButton.y + restartButton.height;
}

int ChessView::getWindowSizeOptionClicked(float x, float y) const {
  for (int index = 0; index < ui::Window::kSizePresetCount; ++index) {
    const Rectangle option = getWindowSizeOptionRect(index);
    if (x >= option.x && x <= option.x + option.width && y >= option.y &&
        y <= option.y + option.height) {
      return index;
    }
  }

  return -1;
}

PieceType ChessView::getPromotionOptionClicked(float x, float y) const {
  for (int index = 0; index < ui::Dialog::kPromotionOptionCount; ++index) {
    const Rectangle option = getPromotionOptionRect(index);
    if (x >= option.x && x <= option.x + option.width && y >= option.y &&
        y <= option.y + option.height) {
      return kPromotionOptions[index];
    }
  }
  return PieceType::None;
}

bool ChessView::isWindowSizeDialogCloseClicked(float x, float y) const {
  const Rectangle closeButton = getWindowSizeCloseButtonRect();
  return x >= closeButton.x && x <= closeButton.x + closeButton.width &&
         y >= closeButton.y && y <= closeButton.y + closeButton.height;
}

bool ChessView::isRestartConfirmYesClicked(float x, float y) const {
  const Rectangle yesButton = getRestartConfirmYesButtonRect();
  return x >= yesButton.x && x <= yesButton.x + yesButton.width &&
         y >= yesButton.y && y <= yesButton.y + yesButton.height;
}

bool ChessView::isRestartConfirmNoClicked(float x, float y) const {
  const Rectangle noButton = getRestartConfirmNoButtonRect();
  return x >= noButton.x && x <= noButton.x + noButton.width &&
         y >= noButton.y && y <= noButton.y + noButton.height;
}

bool ChessView::screenToBoardSquare(float x, float y, Position &out) const {
  const Rectangle grid = getBoardGridRect();
  if (x < grid.x || y < grid.y || x >= grid.x + grid.width ||
      y >= grid.y + grid.height) {
    return false;
  }

  const float cellW = grid.width / static_cast<float>(ui::Board::kSquaresPerSide);
  const float cellH = grid.height / static_cast<float>(ui::Board::kSquaresPerSide);
  const int displayCol = static_cast<int>((x - grid.x) / cellW);
  const int displayRow = static_cast<int>((y - grid.y) / cellH);

  const int col = isBoardFlipped_ ? (ui::Board::kMaxIndex - displayCol)
                                  : displayCol;
  const int row = isBoardFlipped_ ? (ui::Board::kMaxIndex - displayRow)
                                  : displayRow;

  out = Position{row, col};
  return true;
}

void ChessView::drawPiece(PieceType type, ::ChessColor color, float x, float y,
                          float w, float h, float sizeMultiplier) {
  const auto &textures =
      (color == ChessColor::White) ? whiteTextures_ : blackTextures_;
  const auto &sourceRects =
      (color == ChessColor::White) ? whiteSourceRects_ : blackSourceRects_;
  const auto &centerOffsets =
      (color == ChessColor::White) ? whiteCenterOffsetX_ : blackCenterOffsetX_;
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

  const float scale =
      (w / src.width < h / src.height) ? (w / src.width) : (h / src.height);
  const float dstWidth =
      src.width * scale * ui::Piece::kDefaultSizeFactor * sizeMultiplier;
  const float dstHeight =
      src.height * scale * ui::Piece::kDefaultSizeFactor * sizeMultiplier;
  float autoCenterOffsetX = 0.0f;
  const auto centerOffsetIt = centerOffsets.find(type);
  if (centerOffsetIt != centerOffsets.end()) {
    autoCenterOffsetX =
        centerOffsetIt->second * scale * ui::Piece::kDefaultSizeFactor;
  }

  const Rectangle dst = {
      x + (w - dstWidth) * 0.5f + ui::Piece::kCenterOffsetX + autoCenterOffsetX,
      y + h - dstHeight - ui::Piece::kBottomPadding, dstWidth, dstHeight};
  DrawTexturePro(texture, src, dst, {0.0f, 0.0f}, 0.0f, {255, 255, 255, 255});
}

namespace {

int getInitialCount(PieceType type) {
  switch (type) {
  case PieceType::Pawn:
    return ui::Board::kSquaresPerSide;
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
}

float getBoardPieceScale(PieceType type) {
  switch (type) {
  case PieceType::King:
  case PieceType::Queen:
  case PieceType::Bishop:
    return ui::Piece::kBoardScaleLarge;
  default:
    return 1.0f;
  }
}

float getCapturedPieceSizeFactor(PieceType type) {
  switch (type) {
  case PieceType::King:
  case PieceType::Queen:
  case PieceType::Bishop:
    return ui::Piece::kCapturedScaleLarge;
  default:
    return 1.0f;
  }
}

const PieceType *capturedPieceOrder() {
  static const PieceType order[5] = {
      PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight,
      PieceType::Pawn};
  return order;
}

constexpr int kCapturedPieceOrderCount = 5;

struct CapturedSectionLayout {
  float iconW = 0.0f;
  float iconH = 0.0f;
  float gap = 0.0f;
  int safeMaxPerRow = 1;
  int totalCaptured = 0;
  int rows = 0;
  int contentHeight = 0;
};

CapturedSectionLayout computeCapturedSectionLayout(
    int sectionWidth, const std::map<PieceType, int> &captured, float uiScale,
    float boardWidth, int screenWidth) {
  CapturedSectionLayout layout{};

  float capturedScale = boardWidth / ui::Board::kUiBaseWidth;
  if (capturedScale < ui::RightPanel::kCapturedScaleMin) {
    capturedScale = ui::RightPanel::kCapturedScaleMin;
  }
  if (capturedScale > ui::RightPanel::kCapturedScaleMax) {
    capturedScale = ui::RightPanel::kCapturedScaleMax;
  }

  const bool compactColumns =
      screenWidth <= ui::RightPanel::kCompactColumnsMaxScreenWidth;
  layout.iconW =
      (compactColumns ? ui::RightPanel::kIconSizeCompact
                      : ui::RightPanel::kIconSizeRegular) *
      capturedScale;
  layout.iconH =
      (compactColumns ? ui::RightPanel::kIconSizeCompact
                      : ui::RightPanel::kIconSizeRegular) *
      capturedScale;
  layout.gap =
      (compactColumns ? ui::RightPanel::kIconGapCompact
                      : ui::RightPanel::kIconGapRegular) *
      capturedScale;

  const int maxPerRow =
      (sectionWidth - static_cast<int>(ui::RightPanel::kGridInsetWidth * uiScale)) /
      static_cast<int>(layout.iconW + layout.gap);
  layout.safeMaxPerRow = (maxPerRow > 0) ? maxPerRow : 1;

  const PieceType *pieceOrder = capturedPieceOrder();
  for (int index = 0; index < kCapturedPieceOrderCount; ++index) {
    layout.totalCaptured += captured.at(pieceOrder[index]);
  }

  if (layout.totalCaptured == 0) {
    layout.contentHeight = static_cast<int>(ui::RightPanel::kEmptyHeight * uiScale);
    return layout;
  }

  layout.rows = (layout.totalCaptured + layout.safeMaxPerRow - 1) /
                layout.safeMaxPerRow;
  layout.contentHeight =
      static_cast<int>(ui::RightPanel::kSectionTopPadding * uiScale) +
      layout.rows * static_cast<int>(layout.iconH + layout.gap);
  return layout;
}

} // namespace

void ChessView::drawBoardLayers(const Board &board, const Position *selectedSquare,
                                const std::vector<Move> &legalMoves,
                                const CastlingTween *castlingTween,
                                const DragPreview *dragPreview,
                                const Position *invalidHighlightSquare,
                                const std::vector<CaptureEffect> &burningPieces,
                                const CaptureEffect *captureCounterPopup) {
  if (boardTexture_.id != 0) {
    Rectangle src = {0.0f, 0.0f, static_cast<float>(boardTexture_.width),
                     static_cast<float>(boardTexture_.height)};
    if (isBoardFlipped_) {
      src = {0.0f, static_cast<float>(boardTexture_.height),
             static_cast<float>(boardTexture_.width),
             -static_cast<float>(boardTexture_.height)};
    } else {
      src = {static_cast<float>(boardTexture_.width), 0.0f,
             -static_cast<float>(boardTexture_.width),
             static_cast<float>(boardTexture_.height)};
    }
    DrawTexturePro(boardTexture_, src, getBoardRenderRect(), {0.0f, 0.0f}, 0.0f,
                   {255, 255, 255, 255});
  }

  const Rectangle grid = getBoardGridRect();
  const float cellW = grid.width / static_cast<float>(ui::Board::kSquaresPerSide);
  const float cellH = grid.height / static_cast<float>(ui::Board::kSquaresPerSide);
  const bool hasActiveCastlingTween =
      castlingTween != nullptr && castlingTween->progress < 1.0f;
  const bool hasActiveDragPreview =
      dragPreview != nullptr && dragPreview->type != PieceType::None;

  auto getBurningCaptureCount = [&](Position pos) -> int {
    for (const auto &burningPiece : burningPieces) {
      if (burningPiece.pos == pos) {
        return burningPiece.captureCount;
      }
    }
    return 0;
  };

  for (const auto &move : legalMoves) {
    DrawRectangleRec(getBoardSquareRect(move.to), ui::Highlight::kLegalMove);
  }

  if (hasActiveDragPreview) {
    const float mouseX = dragPreview->mousePos.x;
    const float mouseY = dragPreview->mousePos.y;
    if (mouseX >= grid.x && mouseY >= grid.y && mouseX < grid.x + grid.width &&
        mouseY < grid.y + grid.height) {
      const int hoverDisplayCol = static_cast<int>((mouseX - grid.x) / cellW);
      const int hoverDisplayRow = static_cast<int>((mouseY - grid.y) / cellH);
      DrawRectangleRec(getDisplaySquareRect(hoverDisplayRow, hoverDisplayCol),
                       ui::Highlight::kDragHover);
    }
  }

  for (int row = 0; row < ui::Board::kSquaresPerSide; ++row) {
    for (int col = 0; col < ui::Board::kSquaresPerSide; ++col) {
      const Piece *piece = board.getPieceAt({row, col});
      if (piece == nullptr) {
        continue;
      }

      if (hasActiveDragPreview && row == dragPreview->from.row &&
          col == dragPreview->from.col) {
        continue;
      }

      if (hasActiveCastlingTween && piece->getColor() == castlingTween->color &&
          ((piece->getType() == PieceType::King && row == castlingTween->kingTo.row &&
            col == castlingTween->kingTo.col) ||
           (piece->getType() == PieceType::Rook && row == castlingTween->rookTo.row &&
            col == castlingTween->rookTo.col))) {
        continue;
      }

      const Rectangle squareRect = getBoardSquareRect({row, col});
      const int burningCaptureCount = getBurningCaptureCount({row, col});
      Texture2D activeBurningTexture = burningLoopTexture_;
      if (burningCaptureCount >= ui::BurnEffect::kSecondTierCaptureCount &&
          burningLoop2Texture_.id != 0) {
        activeBurningTexture = burningLoop2Texture_;
      }

      if (activeBurningTexture.id != 0 &&
          burningCaptureCount >= ui::BurnEffect::kStartCaptureCount) {
        int frameCount = ui::BurnEffect::kFrameCount;
        if (activeBurningTexture.width < frameCount) {
          frameCount = 1;
        }
        const int frameWidth = activeBurningTexture.width / frameCount;
        const int frameHeight = activeBurningTexture.height;
        const int frameIndex =
            static_cast<int>(GetTime() * ui::BurnEffect::kFrameSpeed) % frameCount;

        int effectiveCaptureCount = burningCaptureCount;
        if (effectiveCaptureCount > ui::BurnEffect::kMaxEffectiveCaptureCount) {
          effectiveCaptureCount = ui::BurnEffect::kMaxEffectiveCaptureCount;
        }
        const int extraCaptures =
            effectiveCaptureCount - ui::BurnEffect::kStartCaptureCount;

        float burnSizeFactor =
            ui::BurnEffect::kSizeBase +
            ui::BurnEffect::kSizeLinear * static_cast<float>(extraCaptures) +
            ui::BurnEffect::kSizeQuadratic *
                static_cast<float>(extraCaptures * extraCaptures);
        if (burnSizeFactor > ui::BurnEffect::kSizeMax) {
          burnSizeFactor = ui::BurnEffect::kSizeMax;
        }

        float burnLiftFactor =
            ui::BurnEffect::kLiftBase +
            ui::BurnEffect::kLiftLinear * static_cast<float>(extraCaptures) +
            ui::BurnEffect::kLiftFromSize *
                (burnSizeFactor - ui::BurnEffect::kLiftRefSize);
        if (burnLiftFactor > ui::BurnEffect::kLiftMax) {
          burnLiftFactor = ui::BurnEffect::kLiftMax;
        }

        const float effectSize = ((cellW < cellH) ? cellW : cellH) * burnSizeFactor;
        const Rectangle effectSrc = {
            static_cast<float>(frameIndex * frameWidth), 0.0f,
            static_cast<float>(frameWidth), static_cast<float>(frameHeight)};
        const Rectangle effectDst = {
            squareRect.x + (cellW - effectSize) * 0.5f,
            squareRect.y + (cellH - effectSize) * 0.5f - cellH * burnLiftFactor,
            effectSize, effectSize};
        DrawTexturePro(activeBurningTexture, effectSrc, effectDst, {0.0f, 0.0f},
                       0.0f, {255, 255, 255, ui::BurnEffect::kTintAlpha});
      }

      drawPiece(piece->getType(), piece->getColor(), squareRect.x, squareRect.y,
                squareRect.width, squareRect.height,
                getBoardPieceScale(piece->getType()));
    }
  }

  if (captureCounterPopup != nullptr &&
      captureCounterPopup->captureCount >= ui::CapturePopup::kStartCaptureCount) {
    const float popupProgress = clamp01(captureCounterPopup->progress);
    const Rectangle squareRect = getBoardSquareRect(captureCounterPopup->pos);

    int effectiveCaptureCount = captureCounterPopup->captureCount;
    if (effectiveCaptureCount > ui::CapturePopup::kMaxEffectiveCaptureCount) {
      effectiveCaptureCount = ui::CapturePopup::kMaxEffectiveCaptureCount;
    }

    const float intensity =
        1.0f +
        ui::CapturePopup::kIntensityPerCapture *
            static_cast<float>(effectiveCaptureCount -
                               ui::CapturePopup::kStartCaptureCount);
    const bool isOverheatTier =
        captureCounterPopup->captureCount >= ui::BurnEffect::kSecondTierCaptureCount;
    const float easeOut = 1.0f - popupProgress;
    const float burstEase = std::pow(easeOut, ui::CapturePopup::kBurstEasePower);
    const float fadeEase = std::sqrt(easeOut);
    const float moveEase = 1.0f - (easeOut * easeOut * easeOut);
    const float scalePulse =
        1.0f + (ui::CapturePopup::kScalePulseFactor * intensity) * burstEase;
    const float floatUp =
        cellH * (ui::CapturePopup::kFloatBase +
                 ui::CapturePopup::kFloatPerIntensity * intensity) *
        moveEase;

    const unsigned char textAlpha = static_cast<unsigned char>(255.0f * fadeEase);
    const unsigned char boxAlpha =
        static_cast<unsigned char>((ui::CapturePopup::kBoxAlphaBase +
                                    ui::CapturePopup::kBoxAlphaPerIntensity * intensity) *
                                   fadeEase);
    const float textAlphaNorm = static_cast<float>(textAlpha) / 255.0f;

    float shakeOffsetX = 0.0f;
    float shakeOffsetY = 0.0f;
    if (effectiveCaptureCount >= ui::CapturePopup::kShakeStartCaptureCount) {
      const float shakeStrength =
          cellW * ui::CapturePopup::kShakeStrengthBase *
          static_cast<float>(effectiveCaptureCount -
                             ui::CapturePopup::kStartCaptureCount) *
          burstEase;
      const float shakePhase =
          static_cast<float>(GetTime() * ui::CapturePopup::kShakePhaseSpeed);
      shakeOffsetX = std::sin(shakePhase) * shakeStrength;
      shakeOffsetY =
          std::cos(shakePhase * ui::CapturePopup::kShakePhaseYMultiplier) *
          shakeStrength * ui::CapturePopup::kShakeYStrengthMultiplier;
    }

    const char *popupText = TextFormat("x%d", captureCounterPopup->captureCount);
    int popupFontSize =
        static_cast<int>(cellH *
                         (ui::CapturePopup::kFontBase +
                          ui::CapturePopup::kFontIntensityFactor * intensity) *
                         scalePulse);
    if (popupFontSize < ui::CapturePopup::kMinFont) {
      popupFontSize = ui::CapturePopup::kMinFont;
    }

    const int popupTextWidth = MeasureText(popupText, popupFontSize);
    const float padX = cellW * ui::CapturePopup::kPadX;
    const float padY = cellH * ui::CapturePopup::kPadY;
    const float badgeW = static_cast<float>(popupTextWidth) + padX * 2.0f;
    const float badgeH = static_cast<float>(popupFontSize) + padY * 2.0f;
    const Rectangle popupBadge = {
        squareRect.x + cellW - badgeW - cellW * ui::CapturePopup::kBadgeRightInset +
            shakeOffsetX,
        squareRect.y + cellH * ui::CapturePopup::kBadgeTopInset - floatUp +
            shakeOffsetY,
        badgeW, badgeH};

    DrawRectangleRounded(
        popupBadge, ui::CapturePopup::kBadgeRoundness,
        ui::CapturePopup::kBadgeSegments,
        {ui::CapturePopup::kBadgeBackground.r, ui::CapturePopup::kBadgeBackground.g,
         ui::CapturePopup::kBadgeBackground.b, boxAlpha});

    const auto popupBorderColor =
        isOverheatTier
            ? Fade(GetColor(ui::CapturePopup::kOverheatBorderColor), textAlphaNorm)
            : Fade(GetColor(ui::CapturePopup::kBorderColor), textAlphaNorm);
    const auto popupTextColor =
        isOverheatTier
            ? Fade(GetColor(ui::CapturePopup::kOverheatTextColor), textAlphaNorm)
            : Fade(GetColor(ui::CapturePopup::kTextColor), textAlphaNorm);

    DrawRectangleRoundedLinesEx(popupBadge, ui::CapturePopup::kBadgeRoundness,
                                ui::CapturePopup::kBadgeSegments,
                                ui::CapturePopup::kBadgeBorderWidth,
                                popupBorderColor);
    DrawText(popupText, static_cast<int>(popupBadge.x + padX),
             static_cast<int>(popupBadge.y + padY), popupFontSize, popupTextColor);
  }

  if (hasActiveCastlingTween) {
    const float progress = clamp01(castlingTween->progress);
    const Rectangle kingFromRect = getBoardSquareRect(castlingTween->kingFrom);
    const Rectangle kingToRect = getBoardSquareRect(castlingTween->kingTo);
    const Rectangle rookFromRect = getBoardSquareRect(castlingTween->rookFrom);
    const Rectangle rookToRect = getBoardSquareRect(castlingTween->rookTo);

    const float kingX = kingFromRect.x + (kingToRect.x - kingFromRect.x) * progress;
    const float kingY = kingFromRect.y + (kingToRect.y - kingFromRect.y) * progress;
    drawPiece(PieceType::King, castlingTween->color, kingX, kingY, cellW, cellH,
              getBoardPieceScale(PieceType::King));

    const float rookX = rookFromRect.x + (rookToRect.x - rookFromRect.x) * progress;
    const float rookY = rookFromRect.y + (rookToRect.y - rookFromRect.y) * progress;
    drawPiece(PieceType::Rook, castlingTween->color, rookX, rookY, cellW, cellH,
              getBoardPieceScale(PieceType::Rook));
  }

  if (selectedSquare != nullptr) {
    DrawRectangleLinesEx(getBoardSquareRect(*selectedSquare),
                         ui::Highlight::kSelectionLine,
                         ui::Highlight::kSelection);
  }

  if (invalidHighlightSquare != nullptr) {
    const Rectangle invalidRect = getBoardSquareRect(*invalidHighlightSquare);
    DrawRectangleRec(invalidRect, ui::Highlight::kInvalidFill);
    DrawRectangleLinesEx(invalidRect, ui::Highlight::kInvalidLine,
                         ui::Highlight::kInvalidBorder);
  }

  if (hasActiveDragPreview) {
    const float dragX = dragPreview->mousePos.x - cellW * 0.5f;
    const float dragY = dragPreview->mousePos.y - cellH * 0.5f;
    drawPiece(dragPreview->type, dragPreview->color, dragX, dragY, cellW, cellH,
              getBoardPieceScale(dragPreview->type));
  }
}

int ChessView::drawCapturedSection(int sectionX, int sectionY, int sectionWidth,
                                   const char *title, ChessColor capturedColor,
                                   const std::map<PieceType, int> &captured) {
  const float uiScale = getUiScale();
  const CapturedSectionLayout layout =
      computeCapturedSectionLayout(sectionWidth, captured, uiScale,
                                   getBoardRenderRect().width, GetScreenWidth());

  float capturedScale = getBoardRenderRect().width / ui::Board::kUiBaseWidth;
  if (capturedScale < ui::RightPanel::kCapturedScaleMin) {
    capturedScale = ui::RightPanel::kCapturedScaleMin;
  }
  if (capturedScale > ui::RightPanel::kCapturedScaleMax) {
    capturedScale = ui::RightPanel::kCapturedScaleMax;
  }

  const int titleFontSize = static_cast<int>(ui::RightPanel::kTitleFont * capturedScale);
  DrawText(title, sectionX + static_cast<int>(ui::RightPanel::kTitlePadX * uiScale),
           sectionY + static_cast<int>(ui::RightPanel::kTitlePadY * uiScale),
           titleFontSize, ui::Dialog::kTextPrimary);

  if (layout.totalCaptured == 0) {
    DrawText("-", sectionX + static_cast<int>(ui::RightPanel::kEmptyX * uiScale),
             sectionY + static_cast<int>(ui::RightPanel::kCapturedY * uiScale),
             static_cast<int>(ui::RightPanel::kEmptyFont * capturedScale),
             {180, 180, 180, 255});
    return layout.contentHeight;
  }

  float iconX = static_cast<float>(sectionX) + ui::RightPanel::kGridInsetX * uiScale;
  float iconY = static_cast<float>(sectionY) + ui::RightPanel::kCapturedY * uiScale;
  int drawn = 0;
  const PieceType *pieceOrder = capturedPieceOrder();
  for (int pieceIndex = 0; pieceIndex < kCapturedPieceOrderCount; ++pieceIndex) {
    const PieceType type = pieceOrder[pieceIndex];
    const int count = captured.at(type);
    for (int index = 0; index < count; ++index) {
      const float pieceFactor = getCapturedPieceSizeFactor(type);
      const float drawW = layout.iconW * pieceFactor;
      const float drawH = layout.iconH * pieceFactor;
      const float drawX = iconX + (layout.iconW - drawW) * 0.5f;
      const float drawY = iconY + (layout.iconH - drawH) * 0.5f;
      drawPiece(type, capturedColor, drawX, drawY, drawW, drawH);

      drawn += 1;
      if (drawn % layout.safeMaxPerRow == 0) {
        iconX = static_cast<float>(sectionX) + ui::RightPanel::kGridInsetX * uiScale;
        iconY += layout.iconH + layout.gap;
      } else {
        iconX += layout.iconW + layout.gap;
      }
    }
  }

  return layout.contentHeight;
}

int ChessView::getCapturedSectionHeight(
    int sectionWidth, const std::map<PieceType, int> &captured) const {
  return computeCapturedSectionLayout(sectionWidth, captured, getUiScale(),
                                      getBoardRenderRect().width,
                                      GetScreenWidth())
      .contentHeight;
}

void ChessView::drawRightPanel(const Board &board) {
  std::map<PieceType, int> whiteRemaining;
  std::map<PieceType, int> blackRemaining;
  for (int row = 0; row < ui::Board::kSquaresPerSide; ++row) {
    for (int col = 0; col < ui::Board::kSquaresPerSide; ++col) {
      const Piece *piece = board.getPieceAt({row, col});
      if (piece == nullptr) {
        continue;
      }
      const PieceType type = piece->getType();
      if (piece->getColor() == ChessColor::White) {
        whiteRemaining[type] += 1;
      } else {
        blackRemaining[type] += 1;
      }
    }
  }

  std::map<PieceType, int> capturedBlackPieces;
  std::map<PieceType, int> capturedWhitePieces;
  const PieceType *pieceOrder = capturedPieceOrder();
  for (int pieceIndex = 0; pieceIndex < kCapturedPieceOrderCount; ++pieceIndex) {
    const PieceType type = pieceOrder[pieceIndex];
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

  const Rectangle rightPanel = getRightPanelRect();
  const int rightPanelX = static_cast<int>(rightPanel.x);
  const int rightPanelWidth = static_cast<int>(rightPanel.width);
  const int rightPanelHeight = static_cast<int>(rightPanel.height);
  if (rightPanelWidth <= 0) {
    return;
  }

  DrawRectangle(rightPanelX, 0, rightPanelWidth, rightPanelHeight,
                ui::RightPanel::kBackground);
  DrawLine(rightPanelX, 0, rightPanelX, rightPanelHeight, ui::RightPanel::kDivider);

  const Vector2 mousePos = GetMousePosition();
  const Rectangle settingsButton = getSettingsButtonRect();
  const Rectangle rotateButton = getRotateButtonRect();
  const Rectangle restartButton = getRestartButtonRect();
  const bool settingsHovered = CheckCollisionPointRec(mousePos, settingsButton);
  const bool rotateHovered = CheckCollisionPointRec(mousePos, rotateButton);
  const bool restartHovered = CheckCollisionPointRec(mousePos, restartButton);

  const float uiScale = getUiScale();
  const auto drawRoundedIconButton = [uiScale](Rectangle buttonRect, bool hovered) {
    if (hovered) {
      const float boost = ui::IconButtons::kHoverBoost * uiScale;
      buttonRect.x -= boost;
      buttonRect.y -= boost;
      buttonRect.width += boost * 2.0f;
      buttonRect.height += boost * 2.0f;
    }

    DrawRectangleRounded(buttonRect, ui::IconButtons::kRoundness,
                        ui::IconButtons::kSegments,
                        hovered ? ui::IconButtons::kButtonFillHover
                                : ui::IconButtons::kButtonFill);
    DrawRectangleRoundedLinesEx(buttonRect, ui::IconButtons::kRoundness,
                                ui::IconButtons::kSegments,
                                ui::IconButtons::kBorderWidth,
                                hovered ? ui::IconButtons::kButtonBorderHover
                                        : ui::IconButtons::kButtonBorder);
  };

  drawRoundedIconButton(settingsButton, settingsHovered);
  drawRoundedIconButton(rotateButton, rotateHovered);
  drawRoundedIconButton(restartButton, restartHovered);

  const float buttonIconSize = ui::IconButtons::kIconSize * uiScale;

  if (settingIconTexture_.id != 0) {
    const Rectangle iconSrc = {0.0f, 0.0f, static_cast<float>(settingIconTexture_.width),
                               static_cast<float>(settingIconTexture_.height)};
    const Rectangle iconDst = {
        settingsButton.x + (settingsButton.width - buttonIconSize) * 0.5f,
        settingsButton.y + (settingsButton.height - buttonIconSize) * 0.5f,
        buttonIconSize, buttonIconSize};
    DrawTexturePro(settingIconTexture_, iconSrc, iconDst, {0.0f, 0.0f}, 0.0f,
                   {255, 255, 255, 255});
  } else {
    DrawCircleV({settingsButton.x + settingsButton.width * 0.5f,
                 settingsButton.y + settingsButton.height * 0.5f},
                ui::IconButtons::kFallbackSettingsOuterRadius * uiScale,
                {235, 235, 235, 255});
    DrawCircleV({settingsButton.x + settingsButton.width * 0.5f,
                 settingsButton.y + settingsButton.height * 0.5f},
                ui::IconButtons::kFallbackSettingsInnerRadius * uiScale,
                {48, 58, 78, 255});
  }

  if (rotateIconTexture_.id != 0) {
    const Rectangle iconSrc = {0.0f, 0.0f, static_cast<float>(rotateIconTexture_.width),
                               static_cast<float>(rotateIconTexture_.height)};
    const Rectangle iconDst = {
        rotateButton.x + (rotateButton.width - buttonIconSize) * 0.5f,
        rotateButton.y + (rotateButton.height - buttonIconSize) * 0.5f,
        buttonIconSize, buttonIconSize};
    DrawTexturePro(rotateIconTexture_, iconSrc, iconDst, {0.0f, 0.0f}, 0.0f,
                   {255, 255, 255, 255});
  } else {
    DrawText("R",
             static_cast<int>(rotateButton.x +
                              ui::IconButtons::kFallbackRotateLabelX * uiScale),
             static_cast<int>(rotateButton.y +
                              ui::IconButtons::kFallbackRotateLabelY * uiScale),
             static_cast<int>(ui::IconButtons::kIconSize * uiScale),
             ui::Dialog::kTextPrimary);
  }

  if (restartIconTexture_.id != 0) {
    const Rectangle iconSrc = {0.0f, 0.0f, static_cast<float>(restartIconTexture_.width),
                               static_cast<float>(restartIconTexture_.height)};
    const Rectangle iconDst = {
        restartButton.x + (restartButton.width - buttonIconSize) * 0.5f,
        restartButton.y + (restartButton.height - buttonIconSize) * 0.5f,
        buttonIconSize, buttonIconSize};
    DrawTexturePro(restartIconTexture_, iconSrc, iconDst, {0.0f, 0.0f}, 0.0f,
                   {255, 255, 255, 255});
  } else {
    const float cx = restartButton.x + restartButton.width * 0.5f;
    const float cy = restartButton.y + restartButton.height * 0.5f;
    DrawRing({cx, cy}, ui::IconButtons::kRestartRingInner * uiScale,
             ui::IconButtons::kRestartRingOuter * uiScale,
             ui::IconButtons::kRestartRingStartDeg,
             ui::IconButtons::kRestartRingEndDeg,
             ui::IconButtons::kRestartRingSegments, {235, 235, 235, 255});
    DrawTriangle({cx + ui::IconButtons::kRestartArrowA_X * uiScale,
                  cy + ui::IconButtons::kRestartArrowA_Y * uiScale},
                 {cx + ui::IconButtons::kRestartArrowB_X * uiScale,
                  cy + ui::IconButtons::kRestartArrowB_Y * uiScale},
                 {cx + ui::IconButtons::kRestartArrowC_X * uiScale,
                  cy + ui::IconButtons::kRestartArrowC_Y * uiScale},
                 ui::Dialog::kTextPrimary);
  }

  const auto drawCenteredHoverText = [this](Rectangle buttonRect, const char *text) {
    const int tooltipFontSize =
        static_cast<int>(ui::IconButtons::kTooltipFont * getUiScale());
    const int textWidth = MeasureText(text, tooltipFontSize);
    const float buttonCenterX = buttonRect.x + buttonRect.width * 0.5f;
    const int textX = static_cast<int>(buttonCenterX - textWidth * 0.5f);
    const int textY = static_cast<int>(buttonRect.y -
                                       ui::IconButtons::kTooltipYOffset * getUiScale());
    DrawText(text, textX, textY, tooltipFontSize, ui::IconButtons::kTooltipText);
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

  const bool swapCapturedSections = isBoardFlipped_;
  const char *topTitle = swapCapturedSections ? "Black captured" : "White captured";
  const ChessColor topPieceColor =
      swapCapturedSections ? ChessColor::White : ChessColor::Black;
  const std::map<PieceType, int> &topCaptured =
      swapCapturedSections ? capturedWhitePieces : capturedBlackPieces;

  const char *bottomTitle =
      swapCapturedSections ? "White captured" : "Black captured";
  const ChessColor bottomPieceColor =
      swapCapturedSections ? ChessColor::Black : ChessColor::White;
  const std::map<PieceType, int> &bottomCaptured =
      swapCapturedSections ? capturedBlackPieces : capturedWhitePieces;

  const int topSectionY = static_cast<int>(ui::RightPanel::kTopSectionY);
  const int topSectionBottom =
      topSectionY + drawCapturedSection(rightPanelX, topSectionY, rightPanelWidth,
                                        topTitle, topPieceColor, topCaptured);

  const int bottomSectionHeight =
      getCapturedSectionHeight(rightPanelWidth, bottomCaptured);
  int bottomSectionY =
      rightPanelHeight - static_cast<int>(ui::RightPanel::kBottomSectionMargin) -
      bottomSectionHeight;
  if (bottomSectionY <
      topSectionBottom + static_cast<int>(ui::RightPanel::kTopBottomGap)) {
    bottomSectionY =
        topSectionBottom + static_cast<int>(ui::RightPanel::kTopBottomGap);
  }

  drawCapturedSection(rightPanelX, bottomSectionY, rightPanelWidth, bottomTitle,
                      bottomPieceColor, bottomCaptured);
}

void ChessView::drawDialogsAndOverlays(bool showRestartConfirm,
                                       bool showWindowSizeDialog,
                                       GameState gameState,
                                       const ChessColor *winnerColor,
                                       const ChessColor *promotionColor) {
  if (gameState == GameState::Checkmate || gameState == GameState::Stalemate) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), ui::Overlay::kGameOver);

    const ui::AutoLayout::Metrics m =
        ui::AutoLayout::ComputeMetrics(GetScreenWidth(), GetScreenHeight());
    const Rectangle dialog = ui::AutoLayout::GameOverDialogRect(m);
    const float heightScale = dialog.height / ui::GameOverDialog::kHeight;

    const int titleFontSize =
        static_cast<int>(ui::GameOverDialog::kTitleFont * heightScale);
    const int bodyFontSize =
        static_cast<int>(ui::GameOverDialog::kBodyFont * heightScale);
    const int hintFontSize =
        static_cast<int>(ui::GameOverDialog::kHintFont * heightScale);

    const char *titleText =
        (gameState == GameState::Checkmate) ? "Checkmate" : "Stalemate";
    const char *bodyText = "";
    if (gameState == GameState::Checkmate) {
      if (winnerColor != nullptr && *winnerColor == ChessColor::White) {
        bodyText = "White wins";
      } else if (winnerColor != nullptr && *winnerColor == ChessColor::Black) {
        bodyText = "Black wins";
      } else {
        bodyText = "Win";
      }
    } else {
      bodyText = "No legal moves and king is safe";
    }
    const char *hintText = "Press Restart to play again";

    const int titleWidth = MeasureText(titleText, titleFontSize);
    const int bodyWidth = MeasureText(bodyText, bodyFontSize);
    const int hintWidth = MeasureText(hintText, hintFontSize);

    DrawRectangleRounded(dialog, ui::GameOverDialog::kRoundness,
                         ui::GameOverDialog::kRoundSegments,
                         ui::GameOverDialog::kBackground);
    DrawRectangleRoundedLinesEx(dialog, ui::GameOverDialog::kRoundness,
                                ui::GameOverDialog::kRoundSegments,
                                ui::GameOverDialog::kBorderWidth,
                                ui::GameOverDialog::kBorder);

    DrawText(titleText,
             static_cast<int>(dialog.x + (dialog.width - titleWidth) * 0.5f),
             static_cast<int>(dialog.y + ui::GameOverDialog::kTitleY * heightScale),
             titleFontSize,
             (gameState == GameState::Checkmate)
                 ? GetColor(ui::GameOverDialog::kCheckmateTitleColor)
                 : GetColor(ui::GameOverDialog::kStalemateTitleColor));
    DrawText(bodyText,
             static_cast<int>(dialog.x + (dialog.width - bodyWidth) * 0.5f),
             static_cast<int>(dialog.y + ui::GameOverDialog::kBodyY * heightScale),
             bodyFontSize, ui::GameOverDialog::kBody);
    DrawText(hintText,
             static_cast<int>(dialog.x + (dialog.width - hintWidth) * 0.5f),
             static_cast<int>(dialog.y + ui::GameOverDialog::kHintY * heightScale),
             hintFontSize, ui::GameOverDialog::kHint);
  }

  if (showRestartConfirm) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), ui::Overlay::kBase);

    const Rectangle dialog = getRestartConfirmDialogRect();
    const Rectangle yesButton = getRestartConfirmYesButtonRect();
    const Rectangle noButton = getRestartConfirmNoButtonRect();
    const Vector2 mousePos = GetMousePosition();

    const float uiScale = getUiScale();
    const int titleFontSize =
        static_cast<int>(ui::Dialog::kRestartTitleFont * uiScale);
    const int bodyFontSize = static_cast<int>(ui::Dialog::kRestartBodyFont * uiScale);
    const int actionFontSize =
        static_cast<int>(ui::Dialog::kRestartActionFont * uiScale);
    const char *titleText = "Restart game?";
    const char *bodyText = "Current match progress will be lost.";
    const char *yesText = "Yes";
    const char *noText = "No";

    const int titleTextWidth = MeasureText(titleText, titleFontSize);
    const int bodyTextWidth = MeasureText(bodyText, bodyFontSize);
    const int yesTextWidth = MeasureText(yesText, actionFontSize);
    const int noTextWidth = MeasureText(noText, actionFontSize);

    const int titleX = static_cast<int>(
        dialog.x + (dialog.width - static_cast<float>(titleTextWidth)) * 0.5f);
    const int titleY =
        static_cast<int>(dialog.y + ui::Dialog::kRestartTitleY * uiScale);
    const int bodyX = static_cast<int>(
        dialog.x + (dialog.width - static_cast<float>(bodyTextWidth)) * 0.5f);
    const int bodyY =
        static_cast<int>(dialog.y + ui::Dialog::kRestartBodyY * uiScale);

    DrawRectangleRounded(dialog, ui::Dialog::kRoundness,
                         ui::Dialog::kRoundSegments, ui::Dialog::kBackground);
    DrawRectangleRoundedLinesEx(dialog, ui::Dialog::kRoundness,
                                ui::Dialog::kRoundSegments,
                                ui::Dialog::kBorderWidth, ui::Dialog::kBorder);
    DrawText(titleText, titleX, titleY, titleFontSize, ui::Dialog::kTextPrimary);
    DrawText(bodyText, bodyX, bodyY, bodyFontSize, ui::Dialog::kTextSecondary);

    const bool yesHovered = CheckCollisionPointRec(mousePos, yesButton);
    const bool noHovered = CheckCollisionPointRec(mousePos, noButton);

    DrawRectangleRounded(yesButton, ui::ActionButton::kRoundness,
                         ui::ActionButton::kSegments,
                         yesHovered ? ui::ActionButton::kPositiveFillHover
                                    : ui::ActionButton::kPositiveFill);
    DrawRectangleRoundedLinesEx(yesButton, ui::ActionButton::kRoundness,
                                ui::ActionButton::kSegments,
                                ui::ActionButton::kBorderWidth,
                                yesHovered ? ui::ActionButton::kPositiveBorderHover
                                           : ui::ActionButton::kPositiveBorder);
    DrawText(yesText,
             static_cast<int>(yesButton.x +
                              (yesButton.width - static_cast<float>(yesTextWidth)) *
                                  0.5f),
             static_cast<int>(yesButton.y +
                              (yesButton.height - static_cast<float>(actionFontSize)) *
                                  0.5f),
             actionFontSize, ui::ActionButton::kYesText);

    DrawRectangleRounded(noButton, ui::ActionButton::kRoundness,
                         ui::ActionButton::kSegments,
                         noHovered ? ui::ActionButton::kNegativeFillHover
                                   : ui::ActionButton::kNegativeFill);
    DrawRectangleRoundedLinesEx(noButton, ui::ActionButton::kRoundness,
                                ui::ActionButton::kSegments,
                                ui::ActionButton::kBorderWidth,
                                noHovered ? ui::ActionButton::kNegativeBorderHover
                                          : ui::ActionButton::kNegativeBorder);
    DrawText(noText,
             static_cast<int>(noButton.x +
                              (noButton.width - static_cast<float>(noTextWidth)) *
                                  0.5f),
             static_cast<int>(noButton.y +
                              (noButton.height - static_cast<float>(actionFontSize)) *
                                  0.5f),
             actionFontSize, ui::ActionButton::kNoText);
  }

  if (showWindowSizeDialog) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), ui::Overlay::kBase);

    const Rectangle dialog = getWindowSizeDialogRect();
    const Rectangle closeButton = getWindowSizeCloseButtonRect();
    const Vector2 mousePos = GetMousePosition();
    const float uiScale = getUiScale();

    const int titleFontSize = static_cast<int>(ui::Dialog::kWindowTitleFont * uiScale);
    const int optionFontSize =
        static_cast<int>(ui::Dialog::kWindowOptionFont * uiScale);
    const int closeFontSize = static_cast<int>(ui::Dialog::kWindowCloseFont * uiScale);

    DrawRectangleRounded(dialog, ui::Dialog::kRoundness,
                         ui::Dialog::kRoundSegments, ui::Dialog::kBackground);
    DrawRectangleRoundedLinesEx(dialog, ui::Dialog::kRoundness,
                                ui::Dialog::kRoundSegments,
                                ui::Dialog::kBorderWidth, ui::Dialog::kBorder);

    const char *titleText = "Window size";
    const int titleWidth = MeasureText(titleText, titleFontSize);
    DrawText(
        titleText,
        static_cast<int>(dialog.x + (dialog.width - static_cast<float>(titleWidth)) *
                                      0.5f),
        static_cast<int>(dialog.y + ui::Dialog::kWindowSizeTitleY * uiScale),
        titleFontSize, ui::Dialog::kTextPrimary);

    for (int index = 0; index < ui::Window::kSizePresetCount; ++index) {
      const Rectangle optionRect = getWindowSizeOptionRect(index);
      const bool hovered = CheckCollisionPointRec(mousePos, optionRect);

      DrawRectangleRounded(optionRect, ui::OptionButton::kRoundness,
                           ui::OptionButton::kSegments,
                           hovered ? ui::OptionButton::kFillHover
                                   : ui::OptionButton::kFill);
      DrawRectangleRoundedLinesEx(optionRect, ui::OptionButton::kRoundness,
                                  ui::OptionButton::kSegments,
                                  ui::OptionButton::kBorderWidth,
                                  hovered ? ui::OptionButton::kBorderHover
                                          : ui::OptionButton::kBorder);

      const char *sizeLabel = ui::Window::kSizePresets[index].label;
      const int labelWidth = MeasureText(sizeLabel, optionFontSize);
      DrawText(
          sizeLabel,
          static_cast<int>(optionRect.x +
                           (optionRect.width - static_cast<float>(labelWidth)) *
                               0.5f),
          static_cast<int>(optionRect.y +
                           (optionRect.height - static_cast<float>(optionFontSize)) *
                               0.5f),
          optionFontSize, ui::Dialog::kTextPrimary);
    }

    const bool closeHovered = CheckCollisionPointRec(mousePos, closeButton);
    DrawRectangleRounded(closeButton, ui::ActionButton::kRoundness,
                         ui::ActionButton::kSegments,
                         closeHovered ? ui::ActionButton::kNegativeFillHover
                                      : ui::ActionButton::kNegativeFill);
    DrawRectangleRoundedLinesEx(closeButton, ui::ActionButton::kRoundness,
                                ui::ActionButton::kSegments,
                                ui::ActionButton::kBorderWidth,
                                closeHovered ? ui::ActionButton::kNegativeBorderHover
                                             : ui::ActionButton::kNegativeBorder);
    const char *closeText = "X";
    const int closeWidth = MeasureText(closeText, closeFontSize);
    DrawText(
        closeText,
        static_cast<int>(closeButton.x +
                         (closeButton.width - static_cast<float>(closeWidth)) *
                             0.5f),
        static_cast<int>(closeButton.y +
                         (closeButton.height - static_cast<float>(closeFontSize)) *
                             0.5f),
        closeFontSize, ui::Dialog::kCloseText);
  }

  if (promotionColor != nullptr) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), ui::Overlay::kBase);

    const Rectangle dialog = getPromotionDialogRect();
    const Vector2 mousePos = GetMousePosition();
    const float uiScale = getUiScale();
    const int titleFontSize =
        static_cast<int>(ui::Dialog::kPromotionTitleFont * uiScale);

    DrawRectangleRounded(dialog, ui::Dialog::kRoundness,
                         ui::Dialog::kRoundSegments, ui::Dialog::kBackground);
    DrawRectangleRoundedLinesEx(dialog, ui::Dialog::kRoundness,
                                ui::Dialog::kRoundSegments,
                                ui::Dialog::kBorderWidth, ui::Dialog::kBorder);

    const char *titleText = "Choose promotion";
    const int titleWidth = MeasureText(titleText, titleFontSize);
    DrawText(
        titleText,
        static_cast<int>(dialog.x + (dialog.width - static_cast<float>(titleWidth)) *
                                      0.5f),
        static_cast<int>(dialog.y + ui::Dialog::kPromotionTitleY * uiScale),
        titleFontSize, ui::Dialog::kTextPrimary);

    for (int index = 0; index < ui::Dialog::kPromotionOptionCount; ++index) {
      const Rectangle optionRect = getPromotionOptionRect(index);
      const bool hovered = CheckCollisionPointRec(mousePos, optionRect);

      DrawRectangleRounded(optionRect, ui::OptionButton::kRoundness,
                           ui::OptionButton::kSegments,
                           hovered ? ui::OptionButton::kFillHover
                                   : ui::OptionButton::kFill);
      DrawRectangleRoundedLinesEx(optionRect, ui::OptionButton::kRoundness,
                                  ui::OptionButton::kSegments,
                                  ui::OptionButton::kBorderWidth,
                                  hovered ? ui::OptionButton::kBorderHover
                                          : ui::OptionButton::kBorder);

      drawPiece(kPromotionOptions[index], *promotionColor, optionRect.x,
                optionRect.y,
                optionRect.width, optionRect.height,
                ui::Dialog::kPromotionPieceScale);
    }
  }
}

void ChessView::update(const GameEvent &event) {
  (void)event;
}

void ChessView::drawBoard(const Board &board, const Position *selectedSquare,
                          const std::vector<Move> &legalMoves,
                          bool showRestartConfirm, bool showWindowSizeDialog,
                          GameState gameState, const ChessColor *winnerColor,
                          const CastlingTween *castlingTween,
                          const DragPreview *dragPreview,
                          const ChessColor *promotionColor,
                          const Position *invalidHighlightSquare,
                          const std::vector<CaptureEffect> &burningPieces,
                          const CaptureEffect *captureCounterPopup) {
  BeginDrawing();
  ClearBackground({0, 0, 0, 255});

  drawBoardLayers(board, selectedSquare, legalMoves, castlingTween, dragPreview,
                  invalidHighlightSquare, burningPieces, captureCounterPopup);
  drawRightPanel(board);
  drawDialogsAndOverlays(showRestartConfirm, showWindowSizeDialog, gameState,
                         winnerColor, promotionColor);

  EndDrawing();
}

void ChessView::update(const Board &board, const Position *selectedSquare,
                       const std::vector<Move> &legalMoves,
                       bool showRestartConfirm, bool showWindowSizeDialog,
                       GameState gameState, const ChessColor *winnerColor,
                       const CastlingTween *castlingTween,
                       const DragPreview *dragPreview,
                       const ChessColor *promotionColor,
                       const Position *invalidHighlightSquare,
                       const std::vector<CaptureEffect> &burningPieces,
                       const CaptureEffect *captureCounterPopup) {
  drawBoard(board, selectedSquare, legalMoves, showRestartConfirm,
            showWindowSizeDialog, gameState, winnerColor, castlingTween,
            dragPreview, promotionColor, invalidHighlightSquare, burningPieces,
            captureCounterPopup);
}
