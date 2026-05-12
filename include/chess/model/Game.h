#ifndef GAME_H
#define GAME_H

#include "chess/model/Types.h"
#include "chess/model/Move.h"
#include "chess/model/Board.h"
#include "chess/model/Observer.h"
#include <optional>
#include <map>
#include <vector>

class Game {
private:
  Board board_;
  ChessColor currentTurn_ = ChessColor::White;
  GameState state_ = GameState::Playing;
  std::vector<Move> moveHistory_;
  std::map<PieceType, int> capturedWhitePieces_;
  std::map<PieceType, int> capturedBlackPieces_;
  std::vector<Observer *> observers_;

  struct GameStateSnapshot {
    Board board;
    ChessColor currentTurn;
    GameState state;
    bool castlingRights[4];
    float whiteTimeLeft;
    float blackTimeLeft;
    std::map<PieceType, int> capturedWhitePieces;
    std::map<PieceType, int> capturedBlackPieces;
  };
  std::vector<GameStateSnapshot> undoStack_;

  // Castling rights: [White kingside, White queenside, Black kingside, Black
  // queenside]
  bool castlingRights_[4] = {true, true, true, true};

  // Timer variables
  int timeControlMinutes_ = 5;
  float whiteTimeLeft_ = 5.0f * 60.0f;
  float blackTimeLeft_ = 5.0f * 60.0f;

  // Internal helpers
  bool wouldBeInCheck(const Move &move, ChessColor color) const;
  void updateCastlingRights(const Move &move);
  void updateGameState();
  void notify(const GameEvent &event);

public:
  Game();

  // State queries
  GameState getState() const { return state_; }
  ChessColor getCurrentTurn() const { return currentTurn_; }
  const Board &getBoard() const { return board_; }
  std::vector<Move> getMoveHistory() const { return moveHistory_; }

  const std::map<PieceType, int> &getCapturedWhitePieces() const { return capturedWhitePieces_; }
  const std::map<PieceType, int> &getCapturedBlackPieces() const { return capturedBlackPieces_; }

  void setTimeControl(int minutes);
  float getWhiteTimeLeft() const { return whiteTimeLeft_; }
  float getBlackTimeLeft() const { return blackTimeLeft_; }
  void tickTimer(float dt);

  // Castling rights access
  bool canCastleKingside(ChessColor color) const;
  bool canCastleQueenside(ChessColor color) const;

  // Move generation (fully legal — filters out self-check)
  std::vector<Move> getLegalMoves(Position pos) const;
  std::vector<Move> getLegalMovesBetween(Position from, Position to) const;
  bool hasPromotionChoices(Position from, Position to) const;
  std::optional<Move> resolveLegalMove(
      Position from, Position to,
      PieceType promotion = PieceType::None) const;
  std::vector<Move> getAllLegalMoves(ChessColor color) const;

  // Execute a move; returns false if the move is illegal
  bool makeMove(const Move &move);
  void restart();
  
  // Persistence and Undo
  bool saveGame(const std::string &filename) const;
  bool loadGame(const std::string &filename);
  bool undo();

  // Observer pattern
  void attach(Observer *observer);
  void detach(Observer *observer);
};

#endif // GAME_H
