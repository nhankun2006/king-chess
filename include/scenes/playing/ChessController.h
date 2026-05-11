#ifndef CHESSCONTROLLER_H
#define CHESSCONTROLLER_H

#include <optional>
#include <memory>
#include <vector>

#include "chess/players/IPlayerAgent.h"
#include "scenes/playing/ChessView.h"
#include "chess/model/Game.h"

class IdleInteractionState;
class DraggingInteractionState;
class PromotionInteractionState;
class RestartModalInteractionState;
class WindowModalInteractionState;
class BotTurnState;
class ChessControllerState;

class ChessController {
public:
  struct RenderState {
    bool showRestartConfirm = false;
    bool showWindowSizeDialog = false;
    bool showSaveMessage = true;
    std::optional<DragPreview> dragPreview;
    std::optional<ChessColor> promotionColor;
  };

private:
  friend class IdleInteractionState;
  friend class DraggingInteractionState;
  friend class PromotionInteractionState;
  friend class RestartModalInteractionState;
  friend class WindowModalInteractionState;
  friend class BotTurnState;

  Game *game_ = nullptr;
  ChessView *view_ = nullptr;
  std::unique_ptr<IPlayerAgent> whitePlayer_;
  std::unique_ptr<IPlayerAgent> blackPlayer_;
  std::unique_ptr<ChessControllerState> state_;

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

  bool botThinking_ = false;

  // Autosave settings
  bool autosaveOnMove_ = true;
  bool autosavePeriodic_ = false;
  double lastAutosaveTime_ = 0.0;
  float autosaveIntervalSeconds_ = 30.0f;

  void updateSelection(Position pos);
  void clearSelection();
  void stopDragging();
  void triggerInvalidMoveWarning(const std::optional<Position> &fallbackSquare);
  bool applyMove(const Move &move);
  const IPlayerAgent *currentPlayer() const;
  bool isInputBlockedByUi() const;
  bool isHumanVsBotMatch() const;
  bool undoForCurrentMode();
  bool gameIsPlayable() const;
  void setState(std::unique_ptr<ChessControllerState> nextState);

public:
  ChessController(Game &game, ChessView &view,
                  std::unique_ptr<IPlayerAgent> whitePlayer,
                  std::unique_ptr<IPlayerAgent> blackPlayer);
  ~ChessController();

  bool processInput();
  const std::optional<Position> &getSelectedSquare() const;
  const std::vector<Move> &getLegalMoves() const;
  RenderState buildRenderState() const;
};

#endif // CHESSCONTROLLER_H
