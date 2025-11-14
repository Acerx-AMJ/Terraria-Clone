#include "game/gameState.hpp"

// Includes

#include <algorithm>
#include <cmath>
#include "PerlinNoise.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
#include "util/render.hpp"

using namespace std::string_literals;

// Constants

namespace {
   constexpr int SIZE_Y = 500;
   constexpr int SIZE_X = 5000;
}

// Constructors

GameState::GameState() {
   blocks = std::vector<std::vector<Block>>(SIZE_Y, std::vector<Block>(SIZE_X, Block{}));
   siv::PerlinNoise perlin {(siv::PerlinNoise::seed_type)rand()};

   int y = SIZE_Y * 0.5f;
   int rockOffset = 12.f;
   
   for (int x = 0; x < SIZE_X; ++x) {
      if (blocks[y][x].tex) {
         continue;
      }
      float noise = (perlin.octave2D(x * 0.01f, y * 0.01f, 4) + 1.f) / 2.f;

      if (noise < .2f) {
         y -= random(0, 3);
         rockOffset -= random(0, 2);
      } else if (noise < .4) {
         y -= random(0, 2);
         rockOffset -= random(0, 1);
      } else if (noise < .65f) {
         if (random(0, 100) >= 50) {
            y += random(-1, 1);
         }
         if (random(0, 100) >= 80) {
            rockOffset += random(-1, 1);
         }
      } else if (noise < .85f) {
         y += random(0, 2);
         rockOffset += random(0, 1);
      } else {
         y += random(0, 3);
         rockOffset += random(0, 2);
      }

      if (y < SIZE_Y * .3f) {
         y++;
         rockOffset++;
      } else if (y > SIZE_Y * .6f) {
         y--;
         rockOffset--;
      }
      y = std::clamp(y, int(SIZE_Y * .25f), int(SIZE_Y * 65.f));
      rockOffset = std::clamp(rockOffset, 5, 25);
      
      setBlock(blocks[y][x], "grass"s);
      int dirtDepth = 0;

      for (int yy = y + 1; yy < SIZE_Y; ++yy) {
         if (dirtDepth < rockOffset) {
            dirtDepth++;
            setBlock(blocks[yy][x], "dirt"s);
         } else {
            setBlock(blocks[yy][x], "stone"s);
         }
      }
   }

   siv::PerlinNoise sandNoise2 {(siv::PerlinNoise::seed_type)rand()};
   siv::PerlinNoise sandNoise {(siv::PerlinNoise::seed_type)rand()};
   siv::PerlinNoise dirtNoise {(siv::PerlinNoise::seed_type)rand()};

   for (int x = 0; x < SIZE_X; ++x) {
      for (int y = 0; y < SIZE_Y; ++y) {
         if (not blocks[y][x].tex) {
            if (y >= SIZE_Y * .45f) {
               setBlock(blocks[y][x], "water"s);
            }
            continue;
         }

         if ((sandNoise.octave2D(x * 0.004f, y * 0.004f, 4) + 1.f) / 2.f >= .7f) {
            if (blocks[y][x].type == Block::Type::stone) {
               setBlock(blocks[y][x], "sandstone"s);
            } else {
               setBlock(blocks[y][x], "sand"s);
            }
         } else if (blocks[y][x].type != Block::Type::grass) {
            float noise = (dirtNoise.octave2D(x * 0.04f, y * 0.04f, 4) + 1.f) / 2.f;
            if (noise >= .825f) {
               setBlock(blocks[y][x], "clay"s);
            } else if (noise <= .2f) {
               setBlock(blocks[y][x], "dirt"s);
            } else if ((sandNoise2.octave2D(x * 0.04f, y * 0.04f, 4) + 1.f) / 2.f <= .15f) {
               setBlock(blocks[y][x], "sand"s);
            }
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
   float speed = (IsKeyDown(KEY_LEFT_SHIFT) ? 1000.f : 500.f);
   camera.offset.x += (IsKeyDown(KEY_A) - IsKeyDown(KEY_D)) * speed * GetFrameTime();
   camera.offset.y += (IsKeyDown(KEY_W) - IsKeyDown(KEY_S)) * speed * GetFrameTime();

   float wheel = GetMouseWheelMove();
   if (wheel != 0.f) {
      Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
      camera.offset = GetMousePosition();
      camera.target = mouseWorldPos;
      camera.zoom = std::clamp(std::exp(std::log(camera.zoom) + wheel * 0.2f), 1.f, 5000.f);
   }

   if (IsKeyReleased(KEY_ESCAPE)) {
      fadingOut = true;
   }
}

// Other functions

void GameState::render() {
   drawRect(BLUE);
   auto crect = getCameraBounds(camera);
   int drew = 0, onScreen = 0, total = 0;

   BeginMode2D(camera);
   auto maxY = std::min(SIZE_Y, int((crect.y + crect.height)) + 1);
   auto maxX = std::min(SIZE_X, int((crect.x + crect.width)) + 1);

   for (int y = 0; y < SIZE_Y; ++y) {
      for (int x = 0; x < SIZE_X; ++x) {
         total += (blocks[y][x].tex != nullptr);
      }
   }
   for (int y = std::max(0, int(crect.y)); y < maxY; ++y) {
      for (int x = std::max(0, int(crect.x)); x < maxX; ++x) {
         auto& block = blocks[y][x];
         if (block.type == Block::Type::air) {
            continue;
         }

         if (camera.zoom <= 12.5f) {
            int ox = x;
            while (x < maxX and blocks[y][x].id == block.id) {
               ++x;
            }
            DrawRectangle(ox, y, x - ox, 1, getBlockColor(block.id));
            onScreen += x - ox;
            --x;
         } else {
            drawTextureNO(*block.tex, {(float)x, (float)y, 1.f, 1.f});
            onScreen++;
         }
         drew++;
      }
   }
   EndMode2D();
   drawText(getScreenCenter(), ("RECTANGLES RENDERED: "s + std::to_string(drew)).c_str(), 80);
   drawText({getScreenCenter().x, getScreenCenter().y + 60.f}, ("ON SCREEN: "s + std::to_string(onScreen)).c_str(), 50);
   drawText({getScreenCenter().x, getScreenCenter().y + 120.f}, ("BLOCKS: "s + std::to_string(total)).c_str(), 50);
   drawText({getScreenCenter().x, getScreenCenter().y - 120.f}, ("FPS: "s + std::to_string(GetFPS())).c_str(), 30);
}

void GameState::change(States& states) {
   states.push_back(GameState::make());
}
