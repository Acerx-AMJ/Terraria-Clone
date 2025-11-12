#ifndef GAME_GAMESTATE_HPP
#define GAME_GAMESTATE_HPP

// Includes

#include "game/state.hpp"
#include "objs/block.hpp"
#include <vector>

// Game state class

class GameState: public State {
   std::vector<std::vector<Block>> blocks;
   Camera2D camera;

public:
   GameState();
   ~GameState() = default;

   static StatePtr make() {
      return std::make_unique<GameState>();
   }

   // Update

   void update() override;

   // Other functions

   void render() override;
   void change(States& states) override;
};

#endif
