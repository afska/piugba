#ifndef ARROW_H
#define ARROW_H

#include <libgba-sprite-engine/sprites/sprite.h>
#include "utils/SpriteUtils.h";
#include "utils/pool/IPoolable.h";

enum ArrowType { DOWNLEFT, UPLEFT, CENTER, UPRIGHT, DOWNRIGHT };
enum ArrowState { ACTIVE, OUT };
const u32 ARROWS_TOTAL = 5;
const u32 ARROW_CORNER_MARGIN = 4;
const u32 ARROW_MARGIN = 16 + 2;

class Arrow : public IPoolable {
 public:
  Arrow(u32 id, ArrowType type);

  void discard() override;

  void initialize();
  ArrowState update();
  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
  ArrowType type;
  bool flip = false;
};

#endif  // ARROW_H
