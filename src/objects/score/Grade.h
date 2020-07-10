#ifndef GRADE_H
#define GRADE_H

#include <libgba-sprite-engine/sprites/sprite.h>

enum GradeType { SS, GS, S, A, B, C, D, F };

class Grade {
 public:
  Grade(GradeType type, u32 x, u32 y);

  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;
};

#endif  // GRADE_H
