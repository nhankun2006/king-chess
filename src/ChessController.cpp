#include "ChessControllder.h"

void ChessController::updateSelection(Position pos) {
  selectedSquare_ = pos;
  selectedLegalMoves_ = game_->getLegalMoves(pos);
}

void ChessController::clearSelection() {
  selectedSquare_.reset();
  selectedLegalMoves_.clear();
}

void ChessController::run() {
  Sound pieceMoveSound{};
  bool pieceMoveSoundLoaded = false;
  Sound captureSound{};
  bool captureSoundLoaded = false;

  const char *appDir = GetApplicationDirectory();
  const std::vector<std::string> soundCandidates = {
      "assets/sounds/piece_move.mp3", "../assets/sounds/piece_move.mp3",
      "../../assets/sounds/piece_move.mp3",
      (appDir ? std::string(appDir) : std::string()) +
          "assets/sounds/piece_move.mp3",
      (appDir ? std::string(appDir) : std::string()) +
          "../assets/sounds/piece_move.mp3",
      (appDir ? std::string(appDir) : std::string()) +
          "../../assets/sounds/piece_move.mp3"};

  for (const auto &path : soundCandidates) {
    if (path.empty() || !FileExists(path.c_str())) {
      continue;
    }
    pieceMoveSound = LoadSound(path.c_str());
    if (pieceMoveSound.frameCount > 0) {
      pieceMoveSoundLoaded = true;
      break;
    }
  }

  const std::vector<std::string> captureSoundCandidates = {
      "assets/sounds/capture.mp3", "../assets/sounds/capture.mp3",
      "../../assets/sounds/capture.mp3",
      (appDir ? std::string(appDir) : std::string()) +
          "assets/sounds/capture.mp3",
      (appDir ? std::string(appDir) : std::string()) +
          "../assets/sounds/capture.mp3",
      (appDir ? std::string(appDir) : std::string()) +
          "../../assets/sounds/capture.mp3"};

  for (const auto &path : captureSoundCandidates) {
    if (path.empty() || !FileExists(path.c_str())) {
      continue;
    }
    captureSound = LoadSound(path.c_str());
    if (captureSound.frameCount > 0) {
      captureSoundLoaded = true;
      break;
    }
  }

  auto isCaptureMoveBeforeExecute = [&](const Move &move) -> bool {
    if (move.isEnPassant) {
      return true;
    }

    const Piece *sourcePiece = game_->getBoard().getPieceAt(move.from);
    const Piece *targetPiece = game_->getBoard().getPieceAt(move.to);
    if (targetPiece == nullptr) {
      return false;
    }
    if (sourcePiece == nullptr) {
      return true;
    }
    return targetPiece->getColor() != sourcePiece->getColor();
  };

  auto playMoveSound = [&](bool isCapture) {
    if (isCapture) {
      if (captureSoundLoaded) {
        PlaySound(captureSound);
      } else if (pieceMoveSoundLoaded) {
        PlaySound(pieceMoveSound);
      }
      return;
    }
    if (pieceMoveSoundLoaded) {
      PlaySound(pieceMoveSound);
    }
  };

  auto getActiveCastlingTween = [&]() -> std::optional<ChessView::CastlingTween> {
    if (!castlingTween_.has_value()) {
      return std::nullopt;
    }

    ChessView::CastlingTween tween = *castlingTween_;
    const double elapsed = GetTime() - castlingTweenStartTime_;
    tween.progress = static_cast<float>(elapsed / castlingTweenDurationSeconds_);
    if (tween.progress >= 1.0f) {
      castlingTween_.reset();
      return std::nullopt;
    }
    if (tween.progress < 0.0f) {
      tween.progress = 0.0f;
    }
    return tween;
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
    std::optional<ChessView::DragPreview> dragPreview;
    if (isDraggingPiece_ && dragFromSquare_.has_value() &&
        dragPieceType_ != PieceType::None) {
      ChessView::DragPreview preview;
      preview.type = dragPieceType_;
      preview.color = dragPieceColor_;
      preview.from = *dragFromSquare_;
      preview.mousePos = GetMousePosition();
      dragPreview = preview;
    }

    std::optional<ChessView::PromotionPrompt> promotionPrompt;
    if (promotionPromptOpen_) {
      ChessView::PromotionPrompt prompt;
      prompt.color = promotionPromptColor_;
      promotionPrompt = prompt;
    }

    std::optional<Position> invalidHighlight;
    if (invalidHighlightSquare_.has_value()) {
      const double elapsed = GetTime() - invalidHighlightStartTime_;
      if (elapsed <= invalidHighlightDurationSeconds_) {
        invalidHighlight = invalidHighlightSquare_;
      } else {
        invalidHighlightSquare_.reset();
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


    std::optional<ChessView::CaptureCounterPopup> captureCounterPopup;
    if (captureCounterPopupSquare_.has_value() && captureCounterPopupCount_ >= 2) {
      const double elapsed = GetTime() - captureCounterPopupStartTime_;
      const float progress =
          static_cast<float>(elapsed / captureCounterPopupDurationSeconds_);
      if (progress < 1.0f) {
        ChessView::CaptureCounterPopup popup;
        popup.pos = *captureCounterPopupSquare_;
        popup.captureCount = captureCounterPopupCount_;
        popup.progress = (progress < 0.0f) ? 0.0f : progress;
        captureCounterPopup = popup;
      } else {
        captureCounterPopupSquare_.reset();
        captureCounterPopupCount_ = 0;
      }
    }

    view_->drawBoard(game_->getBoard(), selectedSquare_, selectedLegalMoves_,
                     restartConfirmOpen_, windowSizeDialogOpen_,
                     getActiveCastlingTween(), dragPreview, promotionPrompt,
                     invalidHighlight, burningPieces,
                     captureCounterPopup);
  };

  auto triggerInvalidMoveWarning = [&](const std::optional<Position> &fallbackSquare) {
    Position warningSquare = {-1, -1};
    const Color sideToMove = game_->getCurrentTurn();
    const Board &board = game_->getBoard();

    if (board.isInCheck(sideToMove)) {
      warningSquare = board.findKing(sideToMove);
    } else if (fallbackSquare.has_value()) {
      warningSquare = *fallbackSquare;
    } else if (selectedSquare_.has_value()) {
      warningSquare = *selectedSquare_;
    }

    if (warningSquare.row >= 0 && warningSquare.row < 8 &&
        warningSquare.col >= 0 && warningSquare.col < 8) {
      invalidHighlightSquare_ = warningSquare;
      invalidHighlightStartTime_ = GetTime();
    }
  };

  auto stopDragging = [&]() {
    isDraggingPiece_ = false;
    dragFromSquare_.reset();
    dragPieceType_ = PieceType::None;
  };

  while (!WindowShouldClose()) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      const Vector2 mousePos = GetMousePosition();

      if (promotionPromptOpen_) {
        const auto selectedPromotion =
            view_->getPromotionOptionClicked(mousePos.x, mousePos.y);
        if (selectedPromotion.has_value()) {
          for (const auto &move : pendingPromotionMoves_) {
            if (move.promotion == *selectedPromotion) {
              const bool willCapture = isCaptureMoveBeforeExecute(move);
              if (!game_->makeMove(move)) {
                continue;
              }
              playMoveSound(willCapture);
              const int captureCount = updateCaptureStreaks(move, willCapture);
              if (willCapture) {
                if (captureCount >= 2) {
                  int effectiveCaptureCount = captureCount;
                  if (effectiveCaptureCount > 5) {
                    effectiveCaptureCount = 5;
                  }
                  captureCounterPopupSquare_ = move.to;
                  captureCounterPopupCount_ = captureCount;
                  captureCounterPopupDurationSeconds_ =
                      1.00f + 0.08f * static_cast<float>(effectiveCaptureCount - 2);
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
          triggerInvalidMoveWarning(std::nullopt);
        }

        drawFrame();
        continue;
      }

      if (restartConfirmOpen_) {
        if (view_->isRestartConfirmYesClicked(mousePos.x, mousePos.y)) {
          game_->restart();
          pieceCaptureCounts_.clear();
          captureCounterPopupSquare_.reset();
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
          const auto selectedSizeOpt =
              view_->getWindowSizeOptionClicked(mousePos.x, mousePos.y);
          if (selectedSizeOpt.has_value()) {
            switch (*selectedSizeOpt) {
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
        const auto clickedSquareOpt =
            view_->screenToBoardSquare(mousePos.x, mousePos.y);
        if (clickedSquareOpt.has_value()) {
          const Position clickedSquare = *clickedSquareOpt;

          const Piece *clickedPiece = game_->getBoard().getPieceAt(clickedSquare);

          if (clickedPiece && clickedPiece->getColor() == game_->getCurrentTurn()) {
            updateSelection(clickedSquare);
            isDraggingPiece_ = true;
            dragFromSquare_ = clickedSquare;
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
      const auto dropSquareOpt = view_->screenToBoardSquare(mousePos.x, mousePos.y);

      std::optional<Move> attemptedMove;
      std::optional<bool> attemptedMoveIsCapture;
      const Piece *selectedPiece =
          (selectedSquare_.has_value())
              ? game_->getBoard().getPieceAt(*selectedSquare_)
              : nullptr;

      if (dropSquareOpt.has_value() && selectedPiece != nullptr) {
        const Position dropSquare = *dropSquareOpt;
        std::vector<Move> candidateMoves;

        for (const auto &legalMove : selectedLegalMoves_) {
          if (legalMove.to != dropSquare) {
            continue;
          }

          candidateMoves.push_back(legalMove);
        }

        if (!candidateMoves.empty()) {
          if (candidateMoves.size() == 1 || selectedPiece->getType() != PieceType::Pawn) {
            attemptedMove = candidateMoves.front();
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
                attemptedMove.reset();
              } else {
                attemptedMove = candidateMoves.front();
              }
            } else {
              attemptedMove = candidateMoves.front();
            }
          }
        }
      }

      if (!promotionPromptOpen_ && attemptedMove.has_value()) {
        const bool willCapture = isCaptureMoveBeforeExecute(*attemptedMove);
        if (!game_->makeMove(*attemptedMove)) {
          attemptedMove.reset();
          attemptedMoveIsCapture.reset();
        } else {
          attemptedMoveIsCapture = willCapture;
        }
      }

      if (!promotionPromptOpen_ && attemptedMove.has_value()) {
        playMoveSound(attemptedMoveIsCapture.value_or(false));
        const int captureCount =
            updateCaptureStreaks(*attemptedMove,
                                 attemptedMoveIsCapture.value_or(false));
        if (attemptedMoveIsCapture.value_or(false)) {
          if (captureCount >= 2) {
            int effectiveCaptureCount = captureCount;
            if (effectiveCaptureCount > 5) {
              effectiveCaptureCount = 5;
            }
            captureCounterPopupSquare_ = attemptedMove->to;
            captureCounterPopupCount_ = captureCount;
            captureCounterPopupDurationSeconds_ =
                1.00f + 0.08f * static_cast<float>(effectiveCaptureCount - 2);
            captureCounterPopupStartTime_ = GetTime();
          }
        }
        if (attemptedMove->isCastling && selectedPiece != nullptr &&
            selectedPiece->getType() == PieceType::King) {
          ChessView::CastlingTween tween;
          tween.color = selectedPiece->getColor();
          tween.kingFrom = attemptedMove->from;
          tween.kingTo = attemptedMove->to;
          const int rookFromCol = (attemptedMove->to.col == 6) ? 7 : 0;
          const int rookToCol = (attemptedMove->to.col == 6) ? 5 : 3;
          tween.rookFrom = {attemptedMove->from.row, rookFromCol};
          tween.rookTo = {attemptedMove->from.row, rookToCol};
          tween.progress = 0.0f;
          castlingTween_ = tween;
          castlingTweenStartTime_ = GetTime();
        }
      } else if (!promotionPromptOpen_) {
        if (dropSquareOpt.has_value() && selectedSquare_.has_value() &&
            *dropSquareOpt != *selectedSquare_) {
          triggerInvalidMoveWarning(dropSquareOpt);
        }
      }

      if (!promotionPromptOpen_) {
        clearSelection();
      }
      stopDragging();
    }

    drawFrame();
  }

  if (pieceMoveSoundLoaded) {
    UnloadSound(pieceMoveSound);
  }
  if (captureSoundLoaded) {
    UnloadSound(captureSound);
  }
}
