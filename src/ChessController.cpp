#include "ChessControllder.h"

void ChessController::updateSelection(Position pos) {
  delete selectedSquare_;
  selectedSquare_ = new Position(pos);
  selectedLegalMoves_ = game_->getLegalMoves(pos);
}

void ChessController::clearSelection() {
  delete selectedSquare_;
  selectedSquare_ = nullptr;
  selectedLegalMoves_.clear();
}

ChessController::~ChessController() {
  delete selectedSquare_;
  delete castlingTween_;
  delete dragFromSquare_;
  delete invalidHighlightSquare_;
  delete captureCounterPopupSquare_;
}

void ChessController::run() {
  auto getActiveCastlingTween =
      [&](ChessView::CastlingTween &outTween) -> bool {
    if (castlingTween_ == nullptr) {
      return false;
    }

    outTween = *castlingTween_;
    const double elapsed = GetTime() - castlingTweenStartTime_;
    outTween.progress =
        static_cast<float>(elapsed / castlingTweenDurationSeconds_);
    if (outTween.progress >= 1.0f) {
      delete castlingTween_;
      castlingTween_ = nullptr;
      return false;
    }
    if (outTween.progress < 0.0f) {
      outTween.progress = 0.0f;
    }
    return true;
  };

  auto positionKey = [](Position pos) -> int { return pos.row * 8 + pos.col; };

  auto updateCaptureStreaks = [&](const Move &move, bool wasCapture) -> int {
    const int fromKey = positionKey(move.from);
    const int toKey = positionKey(move.to);

    int movingPieceCaptureCount = 0;
    const auto fromIt = pieceCaptureCounts_.find(fromKey);
    if (fromIt != pieceCaptureCounts_.end()) {
      movingPieceCaptureCount = fromIt->second;
      pieceCaptureCounts_.erase(fromIt);
    }

    if (wasCapture) {
      movingPieceCaptureCount += 1;
    }
    pieceCaptureCounts_[toKey] = movingPieceCaptureCount;

    if (move.isCastling) {
      const int rookFromCol = (move.to.col == 6) ? 7 : 0;
      const int rookToCol = (move.to.col == 6) ? 5 : 3;
      const Position rookFrom{move.from.row, rookFromCol};
      const Position rookTo{move.from.row, rookToCol};

      const int rookFromKey = positionKey(rookFrom);
      const int rookToKey = positionKey(rookTo);

      int rookCaptureCount = 0;
      const auto rookIt = pieceCaptureCounts_.find(rookFromKey);
      if (rookIt != pieceCaptureCounts_.end()) {
        rookCaptureCount = rookIt->second;
        pieceCaptureCounts_.erase(rookIt);
      }
      pieceCaptureCounts_[rookToKey] = rookCaptureCount;
    }

    return movingPieceCaptureCount;
  };

  auto drawFrame = [&]() {
    ChessView::DragPreview dragPreviewVal;
    ChessView::DragPreview *dragPreview = nullptr;
    if (isDraggingPiece_ && dragFromSquare_ != nullptr &&
        dragPieceType_ != PieceType::None) {
      dragPreviewVal.type = dragPieceType_;
      dragPreviewVal.color = dragPieceColor_;
      dragPreviewVal.from = *dragFromSquare_;
      dragPreviewVal.mousePos = GetMousePosition();
      dragPreview = &dragPreviewVal;
    }

    ChessView::PromotionPrompt promotionPromptVal;
    ChessView::PromotionPrompt *promotionPrompt = nullptr;
    if (promotionPromptOpen_) {
      promotionPromptVal.color = promotionPromptColor_;
      promotionPrompt = &promotionPromptVal;
    }

    Position *invalidHighlight = nullptr;
    if (invalidHighlightSquare_ != nullptr) {
      const double elapsed = GetTime() - invalidHighlightStartTime_;
      if (elapsed <= invalidHighlightDurationSeconds_) {
        invalidHighlight = invalidHighlightSquare_;
      } else {
        delete invalidHighlightSquare_;
        invalidHighlightSquare_ = nullptr;
      }
    }

    std::vector<ChessView::BurningPieceInfo> burningPieces;
    for (const auto &[key, captureCount] : pieceCaptureCounts_) {
      if (captureCount < 2) {
        continue;
      }
      const Position pos{key / 8, key % 8};
      if (game_->getBoard().getPieceAt(pos) != nullptr) {
        burningPieces.push_back({pos, captureCount});
      }
    }

    ChessView::CaptureCounterPopup captureCounterPopupVal;
    ChessView::CaptureCounterPopup *captureCounterPopup = nullptr;
    if (captureCounterPopupSquare_ != nullptr &&
        captureCounterPopupCount_ >= 2) {
      const double elapsed = GetTime() - captureCounterPopupStartTime_;
      const float progress =
          static_cast<float>(elapsed / captureCounterPopupDurationSeconds_);
      if (progress < 1.0f) {
        captureCounterPopupVal.pos = *captureCounterPopupSquare_;
        captureCounterPopupVal.captureCount = captureCounterPopupCount_;
        captureCounterPopupVal.progress = (progress < 0.0f) ? 0.0f : progress;
        captureCounterPopup = &captureCounterPopupVal;
      } else {
        delete captureCounterPopupSquare_;
        captureCounterPopupSquare_ = nullptr;
        captureCounterPopupCount_ = 0;
      }
    }

    const GameState gameState = game_->getState();
    Color winnerColorVal = Color::White;
    Color *winnerColor = nullptr;
    if (gameState == GameState::Checkmate) {
      winnerColorVal = oppositeColor(game_->getCurrentTurn());
      winnerColor = &winnerColorVal;
    }

    ChessView::CastlingTween activeTween;
    ChessView::CastlingTween *activeTweenPtr = nullptr;
    if (getActiveCastlingTween(activeTween)) {
      activeTweenPtr = &activeTween;
    }

    view_->drawBoard(game_->getBoard(), selectedSquare_, selectedLegalMoves_,
                     restartConfirmOpen_, windowSizeDialogOpen_, gameState,
                     winnerColor, activeTweenPtr, dragPreview, promotionPrompt,
                     invalidHighlight, burningPieces, captureCounterPopup);
  };

  auto triggerInvalidMoveWarning = [&](const Position *fallbackSquare) {
    Position warningSquare = {-1, -1};
    const Color sideToMove = game_->getCurrentTurn();
    const Board &board = game_->getBoard();

    if (board.isInCheck(sideToMove)) {
      warningSquare = board.findKing(sideToMove);
    } else if (fallbackSquare != nullptr) {
      warningSquare = *fallbackSquare;
    } else if (selectedSquare_ != nullptr) {
      warningSquare = *selectedSquare_;
    }

    if (warningSquare.row >= 0 && warningSquare.row < 8 &&
        warningSquare.col >= 0 && warningSquare.col < 8) {
      delete invalidHighlightSquare_;
      invalidHighlightSquare_ = new Position(warningSquare);
      invalidHighlightStartTime_ = GetTime();
    }
  };

  auto stopDragging = [&]() {
    isDraggingPiece_ = false;
    delete dragFromSquare_;
    dragFromSquare_ = nullptr;
    dragPieceType_ = PieceType::None;
  };

  while (!WindowShouldClose()) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      const Vector2 mousePos = GetMousePosition();

      if (promotionPromptOpen_) {
        const PieceType selectedPromotion =
            view_->getPromotionOptionClicked(mousePos.x, mousePos.y);
        if (selectedPromotion != PieceType::None) {
          for (const auto &move : pendingPromotionMoves_) {
            if (move.promotion == selectedPromotion) {
              const bool willCapture =
                  move.isEnPassant ||
                  (game_->getBoard().getPieceAt(move.to) != nullptr);
              if (!game_->makeMove(move)) {
                continue;
              }
              const int captureCount = updateCaptureStreaks(move, willCapture);
              if (willCapture) {
                if (captureCount >= 2) {
                  int effectiveCaptureCount = captureCount;
                  if (effectiveCaptureCount > 5) {
                    effectiveCaptureCount = 5;
                  }
                  delete captureCounterPopupSquare_;
                  captureCounterPopupSquare_ = new Position(move.to);
                  captureCounterPopupCount_ = captureCount;
                  captureCounterPopupDurationSeconds_ =
                      1.00f +
                      0.08f * static_cast<float>(effectiveCaptureCount - 2);
                  captureCounterPopupStartTime_ = GetTime();
                }
              }
              promotionPromptOpen_ = false;
              pendingPromotionMoves_.clear();
              clearSelection();
              stopDragging();
              break;
            }
          }
        } else {
          triggerInvalidMoveWarning(nullptr);
        }

        drawFrame();
        continue;
      }

      if (restartConfirmOpen_) {
        if (view_->isRestartConfirmYesClicked(mousePos.x, mousePos.y)) {
          game_->restart();
          pieceCaptureCounts_.clear();
          delete captureCounterPopupSquare_;
          captureCounterPopupSquare_ = nullptr;
          captureCounterPopupCount_ = 0;
          clearSelection();
          stopDragging();
          restartConfirmOpen_ = false;
        } else if (view_->isRestartConfirmNoClicked(mousePos.x, mousePos.y)) {
          restartConfirmOpen_ = false;
        }
        drawFrame();
        continue;
      }

      if (windowSizeDialogOpen_) {
        if (view_->isWindowSizeDialogCloseClicked(mousePos.x, mousePos.y)) {
          windowSizeDialogOpen_ = false;
        } else {
          const int selectedSizeOpt =
              view_->getWindowSizeOptionClicked(mousePos.x, mousePos.y);
          if (selectedSizeOpt >= 0) {
            switch (selectedSizeOpt) {
            case 0:
              SetWindowSize(700, 512);
              break;
            case 1:
              SetWindowSize(900, 650);
              break;
            case 2:
              SetWindowSize(1100, 780);
              break;
            case 3:
              SetWindowSize(1300, 920);
              break;
            default:
              break;
            }
            windowSizeDialogOpen_ = false;
          }
        }

        drawFrame();
        continue;
      }

      bool handledUiClick = false;

      if (view_->isSettingsButtonClicked(mousePos.x, mousePos.y)) {
        stopDragging();
        windowSizeDialogOpen_ = true;
        handledUiClick = true;
      }

      if (!handledUiClick &&
          view_->isRotateButtonClicked(mousePos.x, mousePos.y)) {
        stopDragging();
        view_->toggleBoardOrientation();
        handledUiClick = true;
      }

      if (!handledUiClick &&
          view_->isRestartButtonClicked(mousePos.x, mousePos.y)) {
        stopDragging();
        windowSizeDialogOpen_ = false;
        restartConfirmOpen_ = true;
        handledUiClick = true;
      }

      if (!handledUiClick) {
        Position clickedSquare;
        if (view_->screenToBoardSquare(mousePos.x, mousePos.y, clickedSquare)) {
          const Piece *clickedPiece =
              game_->getBoard().getPieceAt(clickedSquare);

          if (clickedPiece &&
              clickedPiece->getColor() == game_->getCurrentTurn()) {
            updateSelection(clickedSquare);
            isDraggingPiece_ = true;
            delete dragFromSquare_;
            dragFromSquare_ = new Position(clickedSquare);
            dragPieceType_ = clickedPiece->getType();
            dragPieceColor_ = clickedPiece->getColor();
          } else {
            clearSelection();
            stopDragging();
          }
        } else {
          clearSelection();
          stopDragging();
        }
      }
    }

    if (isDraggingPiece_ && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      const Vector2 mousePos = GetMousePosition();
      Position dropSquare;
      bool hasDropSquare =
          view_->screenToBoardSquare(mousePos.x, mousePos.y, dropSquare);

      Move attemptedMove{};
      bool hasAttemptedMove = false;
      bool attemptedMoveIsCapture = false;
      bool hasAttemptedMoveIsCapture = false;
      const Piece *selectedPiece =
          (selectedSquare_ != nullptr)
              ? game_->getBoard().getPieceAt(*selectedSquare_)
              : nullptr;

      if (hasDropSquare && selectedPiece != nullptr) {
        std::vector<Move> candidateMoves;

        for (const auto &legalMove : selectedLegalMoves_) {
          if (legalMove.to != dropSquare) {
            continue;
          }

          candidateMoves.push_back(legalMove);
        }

        if (!candidateMoves.empty()) {
          if (candidateMoves.size() == 1 ||
              selectedPiece->getType() != PieceType::Pawn) {
            attemptedMove = candidateMoves.front();
            hasAttemptedMove = true;
          } else {
            bool hasPromotionChoices = false;
            for (const auto &move : candidateMoves) {
              if (move.promotion != PieceType::None) {
                hasPromotionChoices = true;
                break;
              }
            }

            if (hasPromotionChoices) {
              promotionPromptOpen_ = true;
              promotionPromptColor_ = selectedPiece->getColor();
              pendingPromotionMoves_.clear();
              for (const auto &move : candidateMoves) {
                if (move.promotion != PieceType::None) {
                  pendingPromotionMoves_.push_back(move);
                }
              }
              if (!pendingPromotionMoves_.empty()) {
                hasAttemptedMove = false;
              } else {
                attemptedMove = candidateMoves.front();
                hasAttemptedMove = true;
              }
            } else {
              attemptedMove = candidateMoves.front();
              hasAttemptedMove = true;
            }
          }
        }
      }

      if (!promotionPromptOpen_ && hasAttemptedMove) {
        const bool willCapture =
            attemptedMove.isEnPassant ||
            (game_->getBoard().getPieceAt(attemptedMove.to) != nullptr);
        if (!game_->makeMove(attemptedMove)) {
          hasAttemptedMove = false;
          hasAttemptedMoveIsCapture = false;
        } else {
          attemptedMoveIsCapture = willCapture;
          hasAttemptedMoveIsCapture = true;
        }
      }

      if (!promotionPromptOpen_ && hasAttemptedMove) {
        const bool wasCapture =
            hasAttemptedMoveIsCapture ? attemptedMoveIsCapture : false;
        const int captureCount =
            updateCaptureStreaks(attemptedMove, wasCapture);
        if (wasCapture) {
          if (captureCount >= 2) {
            int effectiveCaptureCount = captureCount;
            if (effectiveCaptureCount > 5) {
              effectiveCaptureCount = 5;
            }
            delete captureCounterPopupSquare_;
            captureCounterPopupSquare_ = new Position(attemptedMove.to);
            captureCounterPopupCount_ = captureCount;
            captureCounterPopupDurationSeconds_ =
                1.00f + 0.08f * static_cast<float>(effectiveCaptureCount - 2);
            captureCounterPopupStartTime_ = GetTime();
          }
        }
        if (attemptedMove.isCastling && selectedPiece != nullptr &&
            selectedPiece->getType() == PieceType::King) {
          ChessView::CastlingTween tween;
          tween.color = selectedPiece->getColor();
          tween.kingFrom = attemptedMove.from;
          tween.kingTo = attemptedMove.to;
          const int rookFromCol = (attemptedMove.to.col == 6) ? 7 : 0;
          const int rookToCol = (attemptedMove.to.col == 6) ? 5 : 3;
          tween.rookFrom = {attemptedMove.from.row, rookFromCol};
          tween.rookTo = {attemptedMove.from.row, rookToCol};
          tween.progress = 0.0f;
          delete castlingTween_;
          castlingTween_ = new ChessView::CastlingTween(tween);
          castlingTweenStartTime_ = GetTime();
        }
      } else if (!promotionPromptOpen_) {
        if (hasDropSquare && selectedSquare_ != nullptr &&
            !(dropSquare == *selectedSquare_)) {
          triggerInvalidMoveWarning(&dropSquare);
        }
      }

      if (!promotionPromptOpen_) {
        clearSelection();
      }
      stopDragging();
    }

    drawFrame();
  }
}
