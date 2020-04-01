#include "Judge.h"

Judge::Judge() {}

void Judge::onPress(ArrowType type,
                    ObjectPool<Arrow>* arrowPool,
                    Score* score) {
  arrowPool->forEachActive([type, score](Arrow* it) {
    int diff = it->get()->getY() - ARROW_CORNER_MARGIN_Y;

    if (!it->isEnding() && it->type == type &&
        abs(diff) < (int)ARROW_SPEED * 5) {
      it->schedulePress();

      score->update(abs(diff) > (int)ARROW_SPEED * 4
                        ? FeedbackType::GOOD
                        : abs(diff) > (int)ARROW_SPEED * 2
                              ? FeedbackType::GREAT
                              : FeedbackType::PERFECT);
    }
  });
}
