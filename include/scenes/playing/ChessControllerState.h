#ifndef CHESSCONTROLLERSTATE_H
#define CHESSCONTROLLERSTATE_H

class ChessController;

/// Abstract base for the controller interaction-state machine.
/// Each concrete state owns one "mode" of user interaction
/// (idle click, dragging, promotion dialog, restart dialog, settings dialog).
#include <optional>
#include "chess/model/Piece.h"

#include "ChessView.h"

class ChessControllerState {
public:
  virtual ~ChessControllerState() = default;

  /// Process one frame of input.  Returns true when the scene should
  /// transition back to the main menu (e.g. user clicked "Exit").
  virtual bool handleInput(ChessController &ctrl) = 0;

  // Polymorphic UI queries
  virtual bool isRestartModalOpen() const { return false; }
  virtual bool isWindowSizeDialogOpen() const { return false; }
  virtual std::optional<DragPreview> getDragPreview() const { return std::nullopt; }
  virtual std::optional<ChessColor> getPromotionColor() const { return std::nullopt; }
  
  /// Determines if the current state should block the bot from automatically taking its turn
  virtual bool blocksBotInput() const { return false; }
};

#endif // CHESSCONTROLLERSTATE_H
