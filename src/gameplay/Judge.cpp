#include "Judge.h"

const u32 MISS_OFFSET = 8;
const u32 BAD_OFFSET = 6;
const u32 GOOD_OFFSET = 4;
const u32 GREAT_OFFSET = 2;

Judge::Judge(ObjectPool<Arrow>* arrowPool, Score* score) {
  this->arrowPool = arrowPool;
  this->score = score;
}

void Judge::onPress(Arrow* arrow) {
  if (arrow->getIsPressed())
    return;

  int y = arrow->get()->getY();
  u32 diff = (u32)abs(y - ARROW_CORNER_MARGIN_Y);

  if (diff < ARROW_SPEED * MISS_OFFSET) {
    if (diff >= ARROW_SPEED * BAD_OFFSET) {
      score->update(FeedbackType::BAD);
      arrow->markAsPressed();
    } else if (diff >= ARROW_SPEED * GOOD_OFFSET) {
      score->update(FeedbackType::GOOD);
      arrow->markAsPressed();
    } else if (diff >= ARROW_SPEED * GREAT_OFFSET) {
      score->update(FeedbackType::GREAT);
      arrow->press();
    } else {
      score->update(FeedbackType::PERFECT);
      arrow->press();
    }
  }
}

void Judge::onOut(Arrow* arrow) {
  if (!arrow->getIsPressed())
    score->update(FeedbackType::MISS);
  arrowPool->discard(arrow->id);
}
