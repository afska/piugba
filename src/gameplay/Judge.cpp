#include "Judge.h"

Judge::Judge() {}

void Judge::onPress(ArrowType type,
                    ObjectPool<Arrow>* arrowPool,
                    Score* score) {
  arrowPool->forEachActive([type, score](Arrow* it) {
    int diff = it->get()->getY() - ARROW_CORNER_MARGIN_Y;
    u32 absDiff = (u32)abs(diff);

    if (!it->isEnding() && it->type == type && absDiff < ARROW_SPEED * 8) {
      if (absDiff >= ARROW_SPEED * 6) {
        score->update(FeedbackType::BAD);
        it->markAsPressed();
      } else if (absDiff >= ARROW_SPEED * 4) {
        score->update(FeedbackType::GOOD);
        it->markAsPressed();
      } else if (absDiff >= ARROW_SPEED * 2) {
        score->update(FeedbackType::GREAT);
        it->press();
      } else {
        score->update(FeedbackType::PERFECT);
        it->press();
      }
    }
  });
}
