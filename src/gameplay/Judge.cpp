#include "Judge.h"

Judge::Judge() {}

void Judge::onPress(ArrowType type,
                    ObjectPool<Arrow>* arrowPool,
                    Score* score) {
  arrowPool->forEachActive([type, score](Arrow* it) {
    int diff = it->get()->getY() - ARROW_CORNER_MARGIN_Y;
    u32 absDiff = (u32)abs(diff);

    if (!it->isEnding() && it->type == type && absDiff < ARROW_SPEED * 6) {
      it->schedulePress();

      if (absDiff >= ARROW_SPEED * 5)
        score->update(FeedbackType::BAD);
      else if (absDiff >= ARROW_SPEED * 4)
        score->update(FeedbackType::GOOD);
      else if (absDiff >= ARROW_SPEED * 2)
        score->update(FeedbackType::GREAT);
      else
        score->update(FeedbackType::PERFECT);
    }
  });
}
