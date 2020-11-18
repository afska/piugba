#ifndef GRADE_H
#define GRADE_H

#include <libgba-sprite-engine/sprites/sprite.h>

enum GradeType { S, A, B, C, D, F, UNPLAYED };

class Grade {
 public:
  Grade(GradeType type, u32 x, u32 y);

  inline GradeType getType() { return type; }

  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;
  GradeType type;
};

#endif  // GRADE_H
