#include "scenes/playing/ChessController.h"

#include <cstdio>

#include "config/UIConfig.h"
#include "scenes/playing/ChessControllerState.h"
#include "scenes/playing/states/BotTurnState.h"
#include "scenes/playing/states/IdleInteractionState.h"
#include "scenes/playing/states/NetworkWaitState.h"
#include "chess/net/NetworkSession.h"

// Construction / Destruction
ChessController::ChessController(Game &game, ChessView &view,
                                 std::unique_ptr<IPlayerAgent> whitePlayer,
                                 std::unique_ptr<IPlayerAgent> blackPlayer,
                                 const std::string &saveFileName,
                                 NetworkSession *networkSession)
    : game_(&game),
      view_(&view),
      whitePlayer_(std::move(whitePlayer)),
      blackPlayer_(std::move(blackPlayer)),
      state_(std::make_unique<IdleInteractionState>()),
      networkSession_(networkSession),
      saveFileName_(saveFileName) {}

ChessController::~ChessController() = default;

// State machine
void ChessController::setState(std::unique_ptr<ChessControllerState> nextState) {
  state_ = std::move(nextState);
}

// Selection helpers

void ChessController::updateSelection(Position pos) {
  selectedSquare_ = pos;
  selectedLegalMoves_ = game_->getLegalMoves(pos);
}

void ChessController::clearSelection() {
  selectedSquare_.reset();
  selectedLegalMoves_.clear();
}

void ChessController::stopDragging() {
  // Dragging state is now handled polymorphically; 
  // simply changing state cleans up the drag variables implicitly.
}

// Move execution
bool ChessController::applyMove(const Move &move) {
  const bool willCapture =
      move.isEnPassant ||
      (game_->getBoard().getPieceAt(move.to) != nullptr);

  if (!game_->makeMove(move)) {
    return false;
  }

  view_->onMoveApplied(move, willCapture);

  // In a network game, send the move to the remote peer.
  // We send after makeMove succeeds so the remote side only sees valid moves.
  if (networkSession_ && networkSession_->isConnected()) {
    networkSession_->sendMove(move);
  }

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
    if (game_->saveGame(saveFileName_)) {
      view_->triggerSaveMessage();
    }
  }

  return true;
}

// Player agent helpers
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
  return state_ && state_->blocksBotInput();
}

bool ChessController::undoForCurrentMode() {
  // Undo is not allowed in LAN games — prevents state desync
  if (isNetworkGame()) return false;

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

bool ChessController::gameIsPlayable() const {
  const GameState gs = game_->getState();
  return gs != GameState::Checkmate && gs != GameState::Stalemate &&
         gs != GameState::Draw;
}

bool ChessController::isNetworkGame() const {
  return networkSession_ != nullptr;
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
  // Tick the game timer
  const float dt = GetFrameTime();
  game_->tickTimer(dt);

  // Quick-save hotkey
  if (IsKeyPressed(KEY_S)) {
    if (game_->saveGame(saveFileName_)) {
      view_->triggerSaveMessage();
    }
  }

  // Periodic autosave
  if (autosavePeriodic_) {
    const double now = GetTime();
    if (now - lastAutosaveTime_ >= autosaveIntervalSeconds_) {
      if (game_->saveGame(saveFileName_)) {
        view_->triggerSaveMessage();
      }
      lastAutosaveTime_ = now;
    }
  }

  // Auto-transition to BotTurnState when it is an automated player's turn
  if (state_ && !state_->isBotThinking() && !isInputBlockedByUi() && gameIsPlayable()) {
    const IPlayerAgent *player = currentPlayer();
    if (player != nullptr && player->isAutomated()) {
      if (isNetworkGame()) {
        setState(std::make_unique<NetworkWaitState>());
      } else {
        setState(std::make_unique<BotTurnState>());
      }
    }
  }

  // Delegate to the active interaction state
  if (state_) {
    return state_->handleInput(*this);
  }

  return false;
}

// Read-only accessors for PlayingScene
const std::optional<Position> &ChessController::getSelectedSquare() const {
  return selectedSquare_;
}

const std::vector<Move> &ChessController::getLegalMoves() const {
  return selectedLegalMoves_;
}

ChessController::RenderState ChessController::buildRenderState() const {
  RenderState rs;
  if (state_) {
    rs.showRestartConfirm = state_->isRestartModalOpen();
    rs.showWindowSizeDialog = state_->isWindowSizeDialogOpen();
    rs.dragPreview = state_->getDragPreview();
    rs.promotionColor = state_->getPromotionColor();
  }
  return rs;
}
