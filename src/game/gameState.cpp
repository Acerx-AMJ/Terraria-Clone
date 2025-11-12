#include "game/gameState.hpp"

// Includes

#include <algorithm>
#include "FastNoiseLite.h"
#include "mngr/resource.hpp"
#include "util/position.hpp"
#include "util/render.hpp"

using namespace std::string_literals;

// Constants

namespace {
   constexpr int SIZE_Y = 500;
   constexpr int SIZE_X = 500;
}

// Constructors

GameState::GameState() {
   blocks = std::vector<std::vector<Block>>(SIZE_Y, std::vector<Block>(SIZE_X, Block{}));
   
   FastNoiseLite noise;
   noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
   noise.SetSeed(rand());

   for (int x = 0; x < SIZE_X; ++x) {
      for (int y = 0; y < SIZE_Y; ++y) {
         if (blocks[y][x].tex) {
            continue;
         }

         if (noise.GetNoise((float)x, (float)y) >= .5f) {
            blocks[y][x].tex = &ResourceManager::get().getTexture("grass"s);
            for (int yy = y + 1; yy < SIZE_Y; ++yy) {
               blocks[yy][x].tex = &ResourceManager::get().getTexture("dirt"s);
            }
            continue;
         }
      }
   }
   camera.target = {0.f, 0.f};
   camera.offset = getScreenCenter();
   camera.rotation = 0.0f;
   camera.zoom = 50.f;
}

// Update functions

void GameState::update() {
   camera.offset.x += (IsKeyDown(KEY_A) - IsKeyDown(KEY_D)) * 500.f * GetFrameTime();
   camera.offset.y += (IsKeyDown(KEY_W) - IsKeyDown(KEY_S)) * 500.f * GetFrameTime();

   float wheel = GetMouseWheelMove();
   if (wheel != 0.f) {
      Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
      camera.offset = GetMousePosition();
      camera.target = mouseWorldPos;
      camera.zoom = std::clamp(std::exp(std::log(camera.zoom) + wheel * 0.2f), 5.f, 5000.f);
   }
}

// Other functions

void GameState::render() {
   drawRect(BLUE);
   auto crect = getCameraBounds(camera);
   int drew = 0;

   BeginMode2D(camera);
   auto maxY = std::min(SIZE_Y, int((crect.y + crect.height)) + 1);
   auto maxX = std::min(SIZE_X, int((crect.x + crect.width)) + 1);

   for (int y = std::max(0, int(crect.y)); y < maxY; ++y) {
      for (int x = std::max(0, int(crect.x)); x < maxX; ++x) {
         auto& block = blocks[y][x];
         if (not block.tex) {
            continue;
         }
         drawTextureNO(*block.tex, {(float)x, (float)y, 1.f, 1.f});
         drew++;
      }
   }
   EndMode2D();
   drawText(getScreenCenter(), ("BLOCKS RENDERED: "s + std::to_string(drew)).c_str(), 100);
   drawText({getScreenCenter().x, getScreenCenter().y - 120.f}, ("FPS: "s + std::to_string(GetFPS())).c_str(), 30);
}

void GameState::change(States& states) {
   
}
