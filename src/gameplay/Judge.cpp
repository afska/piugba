#include "Judge.h"

#include "models/Event.h"

const u32 OFFSET_MISS = 9;
const u32 OFFSET_BAD = 7;
const u32 OFFSET_GOOD = 5;
const u32 OFFSET_GREAT = 3;

Judge::Judge(ObjectPool<Arrow>* arrowPool,
             std::vector<std::unique_ptr<ArrowHolder>>* arrowHolders,
             Score* score,
             std::function<void()> onStageBreak) {
  this->arrowPool = arrowPool;
  this->arrowHolders = arrowHolders;
  this->score = score;
  this->onStageBreak = onStageBreak;
}

void Judge::onPress(Arrow* arrow) {
  bool isUnique = arrow->type == ArrowType::UNIQUE;
  if (!isUnique || arrow->getIsPressed())
    return;

  int y = arrow->get()->getY();
  u32 diff = (u32)abs(y - ARROW_CORNER_MARGIN_Y);

  if (diff < ARROW_SPEED * OFFSET_MISS) {
    if (diff >= ARROW_SPEED * OFFSET_BAD)
      onResult(arrow, FeedbackType::BAD);
    else if (diff >= ARROW_SPEED * OFFSET_GOOD)
      onResult(arrow, FeedbackType::GOOD);
    else if (diff >= ARROW_SPEED * OFFSET_GREAT)
      onResult(arrow, FeedbackType::GREAT);
    else
      onResult(arrow, FeedbackType::PERFECT);
  }
}

void Judge::onOut(Arrow* arrow) {
  bool isUnique = arrow->type == ArrowType::UNIQUE;
  if (isUnique && !arrow->getIsPressed()) {
    FeedbackType result = onResult(arrow, FeedbackType::MISS);
    if (result == FeedbackType::UNKNOWN)
      return;
  }

  arrow->forAll(arrowPool,
                [this](Arrow* arrow) { arrowPool->discard(arrow->id); });
}

void Judge::onHoldTick(u8 arrows, bool canMiss) {
  bool isPressed = true;

  for (u32 i = 0; i < ARROWS_TOTAL; i++) {
    if (arrows & EVENT_ARROW_MASKS[i] && !arrowHolders->at(i)->getIsPressed()) {
      isPressed = false;
      break;
    }
  }

  if (isPressed)
    updateScore(FeedbackType::PERFECT);
  else if (canMiss)
    updateScore(FeedbackType::MISS);
}

FeedbackType Judge::onResult(Arrow* arrow, FeedbackType partialResult) {
  FeedbackType result = arrow->getResult(partialResult, arrowPool);

  if (result != FeedbackType::UNKNOWN) {
    updateScore(result);

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

void Judge::updateScore(FeedbackType result) {
  bool isAlive = score->update(result);
  if (!isAlive)
    this->onStageBreak();
}
