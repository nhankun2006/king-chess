#ifndef CHESSVIEW_H
#define CHESSVIEW_H

#include <map>
#include <string>
#include <vector>

#include "Observer.h"
#include "Types.h"

#include <raylib.h>

#include "Board.h"
#include "Move.h"

// ─── View-related data structs ──────────────────────────────────────────────

struct CastlingTween {
  ChessColor color = ChessColor::White;
  Position kingFrom{};
  Position kingTo{};
  Position rookFrom{};
  Position rookTo{};
  float progress = 1.0f;
};

struct DragPreview {
  PieceType type = PieceType::None;
  ChessColor color = ChessColor::White;
  Position from{};
  Vector2 mousePos{0.0f, 0.0f};
};

struct CaptureEffect {
  Position pos{};
  int captureCount = 0;
  float progress = 0.0f;
};

// ─── ChessView ──────────────────────────────────────────────────────────────

class ChessView : public Observer {
private:
  Texture2D boardTexture_{};
  Texture2D settingIconTexture_{};
  Texture2D rotateIconTexture_{};
  Texture2D restartIconTexture_{};
  Texture2D burningLoopTexture_{};
  Texture2D burningLoop2Texture_{};
  std::map<PieceType, Texture2D> whiteTextures_;
  std::map<PieceType, Texture2D> blackTextures_;
  std::map<PieceType, Rectangle> whiteSourceRects_;
  std::map<PieceType, Rectangle> blackSourceRects_;
  std::map<PieceType, float> whiteCenterOffsetX_;
  std::map<PieceType, float> blackCenterOffsetX_;
  bool isBoardFlipped_ = true;
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
  Rectangle getPromotionDialogRect() const;
  Rectangle getPromotionOptionRect(int index) const;

public:
  ChessView() = default;
  ~ChessView();

  bool LoadAssets();
  const std::string &getLastAssetError() const { return lastAssetError_; }
  void toggleBoardOrientation() { isBoardFlipped_ = !isBoardFlipped_; }
  bool isSettingsButtonClicked(float x, float y) const;
  bool isRotateButtonClicked(float x, float y) const;
  bool isRestartButtonClicked(float x, float y) const;
  bool isRestartConfirmYesClicked(float x, float y) const;
  bool isRestartConfirmNoClicked(float x, float y) const;
  int getWindowSizeOptionClicked(float x, float y) const;
  bool isWindowSizeDialogCloseClicked(float x, float y) const;
  PieceType getPromotionOptionClicked(float x, float y) const;
  bool screenToBoardSquare(float x, float y, Position &out) const;
  void drawPiece(PieceType type, ChessColor color, float x, float y, float w,
                 float h, float sizeMultiplier = 1.0f);
  void drawBoard(const Board &board, const Position *selectedSquare = nullptr,
                 const std::vector<Move> &legalMoves = {},
                 bool showRestartConfirm = false,
                 bool showWindowSizeDialog = false,
                 GameState gameState = GameState::Playing,
                 const ChessColor *winnerColor = nullptr,
                 const CastlingTween *castlingTween = nullptr,
                 const DragPreview *dragPreview = nullptr,
                 const ChessColor *promotionColor = nullptr,
                 const Position *invalidHighlightSquare = nullptr,
                 const std::vector<CaptureEffect> &burningPieces = {},
                 const CaptureEffect *captureCounterPopup = nullptr);

  void onMoveMade(Position from, Position to, bool isCapture) override {
    (void)from;
    (void)to;
    (void)isCapture;
  }

  void onCheck(ChessColor color) override { (void)color; }

  void onCheckmate(ChessColor color) override { (void)color; }

  void onStalemate() override {}

  void onDraw() override {}
};

#endif // CHESSVIEW_H
