#ifndef CHESSVIEW_H
#define CHESSVIEW_H

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "chess/model/Types.h"

#include <raylib.h>

#include "chess/model/Board.h"
#include "chess/model/Move.h"
#include "chess/model/Observer.h"

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
  std::optional<CastlingTween> castlingTween_;
  double castlingTweenStartTime_ = 0.0;
  float castlingTweenDurationSeconds_ = 0.0f;
  std::optional<Position> invalidHighlightSquare_;
  double invalidHighlightStartTime_ = 0.0;
  float invalidHighlightDurationSeconds_ = 0.0f;
  std::map<int, int> pieceCaptureCounts_;
  std::optional<Position> captureCounterPopupSquare_;
  int captureCounterPopupCount_ = 0;
  double captureCounterPopupStartTime_ = 0.0;
  float captureCounterPopupDurationSeconds_ = 0.0f;
  double saveMessageStartTime_ = -100.0;
  float saveMessageDurationSeconds_ = 1.2f;
  bool isBoardFlipped_ = true;
  std::string lastAssetError_;

  Rectangle getBoardRenderRect() const;
  Rectangle getRightPanelRect() const;
  float getUiScale() const;
  Rectangle getSettingsButtonRect() const;
  Rectangle getBoardGridRect() const;
  Rectangle getRotateButtonRect() const;
  Rectangle getRestartButtonRect() const;
  Rectangle getUndoButtonRect() const;
  Rectangle getRestartConfirmDialogRect() const;
  Rectangle getRestartConfirmYesButtonRect() const;
  Rectangle getRestartConfirmNoButtonRect() const;
  Rectangle getWindowSizeDialogRect() const;
  Rectangle getWindowSizeOptionRect(int index) const;
  Rectangle getWindowSizeCloseButtonRect() const;
  Rectangle getExitToMenuButtonRect() const;
  Rectangle getPromotionDialogRect() const;
  Rectangle getPromotionOptionRect(int index) const;
  int boardToDisplayIndex(int boardIndex) const;
  Rectangle getDisplaySquareRect(int displayRow, int displayCol) const;
  Rectangle getBoardSquareRect(Position boardPos) const;
  static int positionKey(Position pos);
  static float clamp01(float value);
  bool shouldShowSaveMessage() const;

  void drawBoardLayers(const Board &board, const Position *selectedSquare,
                       const std::vector<Move> &legalMoves,
                       const CastlingTween *castlingTween,
                       const DragPreview *dragPreview,
                       const Position *invalidHighlightSquare,
                       const std::vector<CaptureEffect> &burningPieces,
                       const CaptureEffect *captureCounterPopup);
  void drawRightPanel(const Board &board, float whiteTimeLeft = 0.0f, float blackTimeLeft = 0.0f, ChessColor currentTurn = ChessColor::White);
  void drawDialogsAndOverlays(bool showRestartConfirm,
                              bool showWindowSizeDialog, GameState gameState,
                              const ChessColor *winnerColor,
                              const ChessColor *promotionColor);
  int drawCapturedSection(int sectionX, int sectionY, int sectionWidth,
                          const char *title, ChessColor capturedColor,
                          const std::map<PieceType, int> &captured);
  int getCapturedSectionHeight(int sectionWidth,
                               const std::map<PieceType, int> &captured) const;

public:
  const CastlingTween *getActiveCastlingTween();
  const Position *getActiveInvalidHighlightSquare();
  const CaptureEffect *getActiveCaptureCounterPopup(CaptureEffect &popupOut);
  std::vector<CaptureEffect> collectBurningPieces(const Board &board) const;
  ChessView();
  ~ChessView();

  bool LoadAssets();
  const std::string &getLastAssetError() const { return lastAssetError_; }
  void toggleBoardOrientation() { isBoardFlipped_ = !isBoardFlipped_; }
  bool isSettingsButtonClicked(float x, float y) const;
  bool isRotateButtonClicked(float x, float y) const;
  bool isRestartButtonClicked(float x, float y) const;
  bool isUndoButtonClicked(float x, float y) const;
  bool isRestartConfirmYesClicked(float x, float y) const;
  bool isRestartConfirmNoClicked(float x, float y) const;
  int getWindowSizeOptionClicked(float x, float y) const;
  bool isWindowSizeDialogCloseClicked(float x, float y) const;
  bool isExitToMenuButtonClicked(float x, float y) const;
  PieceType getPromotionOptionClicked(float x, float y) const;
  bool screenToBoardSquare(float x, float y, Position &out) const;
  void drawPiece(PieceType type, ChessColor color, float x, float y, float w,
                 float h, float sizeMultiplier = 1.0f);
  void triggerCastlingTween(ChessColor color, Position kingFrom, Position kingTo,
                            Position rookFrom, Position rookTo);
  void triggerInvalidHighlight(Position square);
  void onMoveApplied(const Move &move, bool wasCapture);
  void clearCaptureEffects();
  void triggerSaveMessage();
  void resetVisualEffects();
  void update(const GameEvent &event) override;

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
                 const CaptureEffect *captureCounterPopup = nullptr,
                 float whiteTimeLeft = 0.0f,
                 float blackTimeLeft = 0.0f,
                 ChessColor currentTurn = ChessColor::White);

};

#endif // CHESSVIEW_H
