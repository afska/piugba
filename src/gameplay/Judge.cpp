#include "Judge.h"

const u32 MISS_OFFSET = 8;
const u32 BAD_OFFSET = 6;
const u32 GOOD_OFFSET = 4;
const u32 GREAT_OFFSET = 2;

Judge::Judge() {}

void Judge::onPress(ArrowType type,
                    ObjectPool<Arrow>* arrowPool,
                    Score* score) {
  IFTEST return;

  arrowPool->forEachActive([type, score](Arrow* it) {
    int diff = it->get()->getY() - ARROW_CORNER_MARGIN_Y;
    u32 absDiff = (u32)abs(diff);

    if (!it->isEnding() && it->type == type &&
        absDiff < ARROW_SPEED * MISS_OFFSET) {
      if (absDiff >= ARROW_SPEED * BAD_OFFSET) {
        score->update(FeedbackType::BAD);
        it->markAsPressed();
      } else if (absDiff >= ARROW_SPEED * GOOD_OFFSET) {
        score->update(FeedbackType::GOOD);
        it->markAsPressed();
      } else if (absDiff >= ARROW_SPEED * GREAT_OFFSET) {
        score->update(FeedbackType::GREAT);
        it->press();
      } else {
        score->update(FeedbackType::PERFECT);
        it->press();
      }
    }
  });
}
