#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "Game.h"

int main(int argc, char *argv[]) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("King Chess", 800, 600, 0);
  if (!window) {
    SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_Log("SDL3 initialized successfully!");

  // ── Initialize chess game ───────────────────────────────────────────
  Game game;
  SDL_Log("Initial board: %s", game.getBoard().toFEN().c_str());
  SDL_Log("Current turn: %s",
          game.getCurrentTurn() == Color::White ? "White" : "Black");

  // Simple event loop — close on quit
  bool running = true;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }
    }
    SDL_Delay(16); // ~60 fps
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
