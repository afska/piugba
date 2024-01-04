#ifndef DIFFICULTY_H
#define DIFFICULTY_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include <vector>

#include "gameplay/models/Chart.h"

class Difficulty {
 public:
  Difficulty(u32 x, u32 y);

  inline DifficultyLevel getValue() { return value; }

  void setValue(DifficultyLevel value);
  void hide();

  void render(std::vector<Sprite*>* sprites);

 private:
  DifficultyLevel value = DifficultyLevel::NORMAL;
  std::unique_ptr<Sprite> leftSprite;
  std::unique_ptr<Sprite> rightSprite;
  u32 x;
  u32 y;
};

#endif  // DIFFICULTY_H
