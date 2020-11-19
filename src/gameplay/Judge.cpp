#include "Judge.h"

#include "models/Event.h"
#include "multiplayer/Syncer.h"

Judge::Judge(ObjectPool<Arrow>* arrowPool,
             std::vector<std::unique_ptr<ArrowHolder>>* arrowHolders,
             std::array<std::unique_ptr<Score>, GAME_MAX_PLAYERS>* scores,
             std::function<void(u8 playerId)> onStageBreak) {
  this->arrowPool = arrowPool;
  this->arrowHolders = arrowHolders;
  this->scores = scores;
  this->onStageBreak = onStageBreak;
}

void Judge::onPress(Arrow* arrow, TimingProvider* timingProvider, int offset) {
  int actualMsecs = timingProvider->getMsecs() + offset;
  int expectedMsecs = arrow->timestamp;
  u32 diff = (u32)abs(actualMsecs - expectedMsecs);

  if (isInsideTimingWindow(diff)) {
    if (diff >= FRAME_MS * getTimingWindowOf(FeedbackType::BAD))
      onResult(arrow, FeedbackType::BAD);
    else if (diff >= FRAME_MS * getTimingWindowOf(FeedbackType::GOOD))
      onResult(arrow, FeedbackType::GOOD);
    else if (diff >= FRAME_MS * getTimingWindowOf(FeedbackType::GREAT))
      onResult(arrow, FeedbackType::GREAT);
    else
      onResult(arrow, FeedbackType::PERFECT);
  }
}

void Judge::onOut(Arrow* arrow) {
  bool isUnique = arrow->type == ArrowType::UNIQUE;
  bool isHoldHead = arrow->type == ArrowType::HOLD_HEAD;

  if (isUnique && !arrow->getIsPressed()) {
    FeedbackType result = onResult(arrow, FeedbackType::MISS);
    if (result == FeedbackType::UNKNOWN)
      return;
  }

  if (isHoldHead) {
    FeedbackType result =
        onResult(arrow, arrow->getIsPressed() ? FeedbackType::PERFECT
                                              : FeedbackType::MISS);
    if (result == FeedbackType::UNKNOWN)
      return;
  }

  arrow->forAll(arrowPool,
                [this](Arrow* arrow) { arrowPool->discard(arrow->id); });
}

void Judge::onHoldTick(u8 arrows, u8 playerId, bool canMiss) {
  bool isPressed = true;

  for (u32 i = 0; i < ARROWS_TOTAL; i++) {
    if (arrows & EVENT_ARROW_MASKS[i] &&
        !this->isPressed(static_cast<ArrowDirection>(i), playerId)) {
      isPressed = false;
      break;
    }
  }

  if (isPressed)
    updateScore(FeedbackType::PERFECT, playerId, true);
  else if (canMiss)
    updateScore(FeedbackType::MISS, playerId, true);
}

FeedbackType Judge::onResult(Arrow* arrow, FeedbackType partialResult) {
  FeedbackType result = arrow->getResult(partialResult, arrowPool);

  if (result != FeedbackType::UNKNOWN) {
    if (!arrow->isFake)
      updateScore(result, arrow->playerId);

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

void Judge::updateScore(FeedbackType result, u8 playerId, bool isLong) {
  if (isDisabled)
    return;

  if (GameState.mods.stageBreak == StageBreakOpts::sSUDDEN_DEATH &&
      result == FeedbackType::MISS) {
    onStageBreak(playerId);
    return;
  }

  if (isMultiplayer()) {
    if (playerId == syncer->getLocalPlayerId())
      syncer->send(SYNC_EVENT_FEEDBACK,
                   SYNC_MSG_FEEDBACK_BUILD(result, isLong));
    else
      return;
  }

  bool isAlive = scores->at(playerId)->update(result, isLong);
  if (!isAlive)
    onStageBreak(playerId);
}
