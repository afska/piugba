#ifndef DIFFICULTY_H
#define DIFFICULTY_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include <vector>

#include "gameplay/models/Chart.h"

class Difficulty {
 public:
  Difficulty();

  void setValue(DifficultyLevel value);
  DifficultyLevel getValue();

  void render(std::vector<Sprite*>* sprites);

 private:
  DifficultyLevel value = DifficultyLevel::NORMAL;
  std::unique_ptr<Sprite> leftSprite;
  std::unique_ptr<Sprite> rightSprite;
};

#endif  // DIFFICULTY_H