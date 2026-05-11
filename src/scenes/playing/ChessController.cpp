#include "scenes/playing/ChessController.h"

#include <cstdio>

#include "config/UIConfig.h"
#include "scenes/playing/ChessControllerState.h"
#include "scenes/playing/states/IdleInteractionState.h"

// ─── Construction / Destruction ─────────────────────────────────────────────

ChessController::ChessController(Game &game, ChessView &view,
                                 std::unique_ptr<IPlayerAgent> whitePlayer,
                                 std::unique_ptr<IPlayerAgent> blackPlayer)
    : game_(&game),
      view_(&view),
      whitePlayer_(std::move(whitePlayer)),
      blackPlayer_(std::move(blackPlayer)),
      state_(std::make_unique<IdleInteractionState>()) {}

ChessController::~ChessController() = default;

// ─── State machine ──────────────────────────────────────────────────────────

void ChessController::setState(std::unique_ptr<ChessControllerState> nextState) {
  state_ = std::move(nextState);
}

// ─── Selection helpers ──────────────────────────────────────────────────────

void ChessController::updateSelection(Position pos) {
  selectedSquare_ = pos;
  selectedLegalMoves_ = game_->getLegalMoves(pos);
}

void ChessController::clearSelection() {
  selectedSquare_.reset();
  selectedLegalMoves_.clear();
}

void ChessController::stopDragging() {
  isDraggingPiece_ = false;
  dragFromSquare_.reset();
  dragPieceType_ = PieceType::None;
}

// ─── Move execution ─────────────────────────────────────────────────────────

bool ChessController::applyMove(const Move &move) {
  const bool willCapture =
      move.isEnPassant ||
      (game_->getBoard().getPieceAt(move.to) != nullptr);

  if (!game_->makeMove(move)) {
    return false;
  }

  view_->onMoveApplied(move, willCapture);

  // Castling rook tween
  const Piece *movedPiece = game_->getBoard().getPieceAt(move.to);
  if (move.isCastling && movedPiece != nullptr &&
      movedPiece->getType() == PieceType::King) {
    const int rookFromCol = (move.to.col == 6) ? 7 : 0;
    const int rookToCol = (move.to.col == 6) ? 5 : 3;
    view_->triggerCastlingTween(movedPiece->getColor(), move.from, move.to,
                                {move.from.row, rookFromCol},
                                {move.from.row, rookToCol});
  }

  // Autosave on move
  if (autosaveOnMove_) {
    if (game_->saveGame("save.bin")) {
      view_->triggerSaveMessage();
    }
  }

  return true;
}

// ─── Player agent helpers ───────────────────────────────────────────────────

const IPlayerAgent *ChessController::currentPlayer() const {
  return (game_->getCurrentTurn() == ChessColor::White)
             ? whitePlayer_.get()
             : blackPlayer_.get();
}

bool ChessController::isHumanVsBotMatch() const {
  return (whitePlayer_ && blackPlayer_) &&
         (whitePlayer_->isAutomated() != blackPlayer_->isAutomated());
}

bool ChessController::isInputBlockedByUi() const {
  return restartConfirmOpen_ || windowSizeDialogOpen_ || promotionPromptOpen_;
}

bool ChessController::undoForCurrentMode() {
  if (isHumanVsBotMatch()) {
    // Undo twice so the human gets their own turn back
    const bool first = game_->undo();
    const bool second = game_->undo();
    if (first || second) {
      view_->clearCaptureEffects();
      clearSelection();
      return true;
    }
    return false;
  }
  if (game_->undo()) {
    view_->clearCaptureEffects();
    clearSelection();
    return true;
  }
  return false;
}

bool ChessController::applyAutomatedMoveIfNeeded() {
  const IPlayerAgent *player = currentPlayer();
  if (player == nullptr || !player->isAutomated()) {
    return false;
  }
  if (isInputBlockedByUi()) {
    return false;
  }
  if (game_->getState() == GameState::Checkmate ||
      game_->getState() == GameState::Stalemate ||
      game_->getState() == GameState::Draw) {
    return false;
  }

  // const_cast because chooseMove is non-const (may update internal state)
  auto optMove =
      const_cast<IPlayerAgent *>(player)->chooseMove(*game_);
  if (optMove.has_value()) {
    return applyMove(optMove.value());
  }
  return false;
}

// ─── Feedback helpers ───────────────────────────────────────────────────────

void ChessController::triggerInvalidMoveWarning(
    const std::optional<Position> &fallbackSquare) {
  Position warningSquare = {-1, -1};
  const ChessColor sideToMove = game_->getCurrentTurn();
  const Board &board = game_->getBoard();

  if (board.isInCheck(sideToMove)) {
    warningSquare = board.findKing(sideToMove);
  } else if (fallbackSquare.has_value()) {
    warningSquare = fallbackSquare.value();
  } else if (selectedSquare_.has_value()) {
    warningSquare = selectedSquare_.value();
  }

  if (warningSquare.row >= 0 && warningSquare.row < 8 &&
      warningSquare.col >= 0 && warningSquare.col < 8) {
    view_->triggerInvalidHighlight(warningSquare);
  }
}

// ─── Main per-frame entry point ─────────────────────────────────────────────

bool ChessController::processInput() {
  // Quick-save hotkey (always available)
  if (IsKeyPressed(KEY_S)) {
    if (game_->saveGame("save.bin")) {
      view_->triggerSaveMessage();
    }
  }

  // Periodic autosave
  if (autosavePeriodic_) {
    const double now = GetTime();
    if (now - lastAutosaveTime_ >= autosaveIntervalSeconds_) {
      if (game_->saveGame("save.bin")) {
        view_->triggerSaveMessage();
      }
      lastAutosaveTime_ = now;
    }
  }

  // Let the bot play if it is its turn
  applyAutomatedMoveIfNeeded();

  // Delegate to the active interaction state
  if (state_) {
    return state_->handleInput(*this);
  }
  return false;
}

// ─── Read-only accessors for PlayingScene ───────────────────────────────────

const std::optional<Position> &ChessController::getSelectedSquare() const {
  return selectedSquare_;
}

const std::vector<Move> &ChessController::getLegalMoves() const {
  return selectedLegalMoves_;
}

ChessController::RenderState ChessController::buildRenderState() const {
  RenderState rs;
  rs.showRestartConfirm = restartConfirmOpen_;
  rs.showWindowSizeDialog = windowSizeDialogOpen_;
  rs.showSaveMessage = true;

  if (isDraggingPiece_ && dragFromSquare_.has_value() &&
      dragPieceType_ != PieceType::None) {
    DragPreview dp;
    dp.type = dragPieceType_;
    dp.color = dragPieceColor_;
    dp.from = dragFromSquare_.value();
    dp.mousePos = GetMousePosition();
    rs.dragPreview = dp;
  }

  if (promotionPromptOpen_) {
    rs.promotionColor = promotionPromptColor_;
  }

  return rs;
}
