#ifndef CHESSCONTROLLER_H
#define CHESSCONTROLLER_H

#include <optional>
#include <vector>

#include "scenes/playing/ChessView.h"
#include "chess/model/Game.h"

class ChessController {
private:
  Game *game_ = nullptr;
  ChessView *view_ = nullptr;

  std::optional<Position> selectedSquare_;
  std::vector<Move> selectedLegalMoves_;

  bool restartConfirmOpen_ = false;
  bool windowSizeDialogOpen_ = false;

  bool isDraggingPiece_ = false;
  std::optional<Position> dragFromSquare_;
  PieceType dragPieceType_ = PieceType::None;
  ChessColor dragPieceColor_ = ChessColor::White;

  bool promotionPromptOpen_ = false;
  ChessColor promotionPromptColor_ = ChessColor::White;
  std::vector<Move> pendingPromotionMoves_;

  // Autosave settings
  bool autosaveOnMove_ = true;
  bool autosavePeriodic_ = false;
  double lastAutosaveTime_ = 0.0;
  float autosaveIntervalSeconds_ = 30.0f;

  void updateSelection(Position pos);
  void clearSelection();
  void stopDragging();
  void triggerInvalidMoveWarning(const std::optional<Position> &fallbackSquare);

public:
  ChessController(Game &game, ChessView &view) : game_(&game), view_(&view) {}
  ~ChessController() = default;

  bool processInput();
  void render();
};

#endif // CHESSCONTROLLER_H
