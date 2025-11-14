#ifndef GAME_GAME_HPP
#define GAME_GAME_HPP

// Includes

#include "game/state.hpp"

// Game

struct Game {
   Game();
   ~Game();

   void run();
   States states;
};

#endif
