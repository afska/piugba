#ifndef GRADE_BADGE_H
#define GRADE_BADGE_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/score/Grade.h"

class GradeBadge {
 public:
  GradeBadge(u32 x, u32 y, bool reuseTiles, bool isEvaluation);

  void setType(GradeType type);

  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;

  u32 x;
  u32 y;
};

#endif  // GRADE_BADGE_H
