#ifndef CHESSCONTROLLER_H
#define CHESSCONTROLLER_H

#include <map>
#include <vector>

#include "Game.h"
#include "ChessView.h"
#include "UIConfig.h"

class ChessController {
private:
  Game *game_ = nullptr;
  ChessView *view_ = nullptr;
  Position *selectedSquare_ = nullptr;
  std::vector<Move> selectedLegalMoves_;
  bool restartConfirmOpen_ = false;
  bool windowSizeDialogOpen_ = false;
  CastlingTween *castlingTween_ = nullptr;
  double castlingTweenStartTime_ = 0.0;
  float castlingTweenDurationSeconds_ = ui::Animation::kCastlingTweenDurationSeconds;
  bool isDraggingPiece_ = false;
  Position *dragFromSquare_ = nullptr;
  PieceType dragPieceType_ = PieceType::None;
  ChessColor dragPieceColor_ = ChessColor::White;
  bool promotionPromptOpen_ = false;
  ChessColor promotionPromptColor_ = ChessColor::White;
  std::vector<Move> pendingPromotionMoves_;
  Position *invalidHighlightSquare_ = nullptr;
  double invalidHighlightStartTime_ = 0.0;
  float invalidHighlightDurationSeconds_ = ui::Animation::kInvalidHighlightDurationSeconds;
  std::map<int, int> pieceCaptureCounts_;
  Position *captureCounterPopupSquare_ = nullptr;
  int captureCounterPopupCount_ = 0;
  double captureCounterPopupStartTime_ = 0.0;
  float captureCounterPopupDurationSeconds_ = ui::Animation::kCapturePopupDurationSeconds;

  void updateSelection(Position pos);
  void clearSelection();

public:
  ChessController(Game &game, ChessView &view) : game_(&game), view_(&view) {}
  ~ChessController();

  void run();
};

#endif // CHESSCONTROLLER_H
