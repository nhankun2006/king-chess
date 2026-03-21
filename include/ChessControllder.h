#ifndef CHESSCONTROLLER_H
#define CHESSCONTROLLER_H

#include <optional>
#include <map>
#include <vector>

#include "Game.h"
#include "ChessView.h"

class ChessController {
private:
  Game *game_ = nullptr;
  ChessView *view_ = nullptr;
  std::optional<Position> selectedSquare_;
  std::vector<Move> selectedLegalMoves_;
  bool restartConfirmOpen_ = false;
  bool windowSizeDialogOpen_ = false;
  std::optional<ChessView::CastlingTween> castlingTween_;
  double castlingTweenStartTime_ = 0.0;
  float castlingTweenDurationSeconds_ = 0.22f;
  bool isDraggingPiece_ = false;
  std::optional<Position> dragFromSquare_;
  PieceType dragPieceType_ = PieceType::None;
  Color dragPieceColor_ = Color::White;
  bool promotionPromptOpen_ = false;
  Color promotionPromptColor_ = Color::White;
  std::vector<Move> pendingPromotionMoves_;
  std::optional<Position> invalidHighlightSquare_;
  double invalidHighlightStartTime_ = 0.0;
  float invalidHighlightDurationSeconds_ = 0.55f;
  std::map<int, int> pieceCaptureCounts_;
  std::optional<Position> captureCounterPopupSquare_;
  int captureCounterPopupCount_ = 0;
  double captureCounterPopupStartTime_ = 0.0;
  float captureCounterPopupDurationSeconds_ = 1.05f;

  void updateSelection(Position pos);
  void clearSelection();

public:
  ChessController(Game &game, ChessView &view) : game_(&game), view_(&view) {}

  void run();
};

#endif // CHESSCONTROLLER_H
