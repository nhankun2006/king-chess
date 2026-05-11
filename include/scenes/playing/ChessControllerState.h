#ifndef CHESSCONTROLLERSTATE_H
#define CHESSCONTROLLERSTATE_H

class ChessController;

/// Abstract base for the controller interaction-state machine.
/// Each concrete state owns one "mode" of user interaction
/// (idle click, dragging, promotion dialog, restart dialog, settings dialog).
class ChessControllerState {
public:
  virtual ~ChessControllerState() = default;

  /// Process one frame of input.  Returns true when the scene should
  /// transition back to the main menu (e.g. user clicked "Exit").
  virtual bool handleInput(ChessController &ctrl) = 0;
};

#endif // CHESSCONTROLLERSTATE_H
