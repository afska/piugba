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
    if (diff >= ARROW_SPEED * BAD_OFFSET)
      onResult(arrow, FeedbackType::BAD);
    else if (diff >= ARROW_SPEED * GOOD_OFFSET)
      onResult(arrow, FeedbackType::GOOD);
    else if (diff >= ARROW_SPEED * GREAT_OFFSET)
      onResult(arrow, FeedbackType::GREAT);
    else
      onResult(arrow, FeedbackType::PERFECT);
  }
}

void Judge::onOut(Arrow* arrow) {
  if (!arrow->getIsPressed()) {
    FeedbackType result = onResult(arrow, FeedbackType::MISS);
    if (result == FeedbackType::UNKNOWN)
      return;
  }

  arrow->forAll(arrowPool,
                [this](Arrow* arrow) { arrowPool->discard(arrow->id); });
}

FeedbackType Judge::onResult(Arrow* arrow, FeedbackType partialResult) {
  FeedbackType result = arrow->getResult(partialResult, arrowPool);

  if (result != FeedbackType::UNKNOWN) {
    score->update(result);

    switch (result) {
      case FeedbackType::MISS:
      case FeedbackType::BAD:
      case FeedbackType::GOOD:
        arrow->forAll(arrowPool, [](Arrow* arrow) { arrow->markAsPressed(); });
        break;
      case FeedbackType::GREAT:
      case FeedbackType::PERFECT:
        arrow->forAll(arrowPool, [](Arrow* arrow) { arrow->press(); });
        break;
      default:
        break;
    }
  }

  return result;
}
