#include "ChartReader.h"

#include <libgba-sprite-engine/gba/tonc_math.h>

#include <vector>

#include "debug/logDebugInfo.h"

const u32 HOLD_ARROW_POOL_SIZE = 10;
const u32 FRAME_SKIP = 1;

ChartReader::ChartReader(Chart* chart,
                         ObjectPool<Arrow>* arrowPool,
                         Judge* judge,
                         PixelBlink* pixelBlink,
                         int audioLag,
                         u32 multiplier) {
  this->chart = chart;
  this->arrowPool = arrowPool;
  this->judge = judge;
  this->pixelBlink = pixelBlink;
  this->audioLag = audioLag;

  holdArrows = std::unique_ptr<ObjectPool<HoldArrow>>{new ObjectPool<HoldArrow>(
      HOLD_ARROW_POOL_SIZE,
      [](u32 id) -> HoldArrow* { return new HoldArrow(id); })};
  for (u32 i = 0; i < ARROWS_TOTAL; i++) {
    holdArrowStates[i].isActive = false;
    holdArrowStates[i].currentStartTime = 0;
    holdArrowStates[i].lastStartTime = 0;
  }

  this->multiplier = multiplier;
  targetArrowTime = ARROW_TIME[multiplier];
  syncArrowTime();
  frameSkipCount = FRAME_SKIP;
};

bool ChartReader::update(int songMsecs) {
  int rythmMsecs = songMsecs - audioLag + offset - lastBpmChange;
  msecs = songMsecs - audioLag + offset - (int)stoppedMs + (int)warpedMs;

  MATH_approximate(&arrowTime, targetArrowTime, maxArrowTimeJump);

  if (hasStopped) {
    if (msecs >= stopStart + (int)stopLength) {
      hasStopped = false;
      stoppedMs += stopLength;
      msecs -= (int)stopLength;
    } else {
      orchestrateHoldArrows();
      return processTicks(rythmMsecs, false);
    }
  }

  processNextEvents();
  orchestrateHoldArrows();
  return processTicks(rythmMsecs, true);
}

int ChartReader::getYFor(Arrow* arrow) {
  HoldArrow* holdArrow = arrow->getHoldArrow();

  int y;
  switch (arrow->type) {
    case ArrowType::HOLD_HEAD: {
      y = getFillTopY(holdArrow) - HOLD_getFirstFillOffset(arrow->direction);
      break;
    }
    case ArrowType::HOLD_FILL: {
      bool isFirst = holdArrow->currentFillOffset == holdArrow->fillOffsetSkip;
      bool isOut = holdArrow->currentFillOffset >= holdArrow->fillOffsetBottom;
      bool isLast = !isOut && holdArrow->hasEndTime() &&
                    holdArrow->currentFillOffset + (int)ARROW_SIZE >=
                        holdArrow->fillOffsetBottom;

      if (isOut)
        return -ARROW_SIZE;

      if (isLast) {
        arrow->get()->setPriority(ARROW_LAYER_FRONT);
        y = getFillBottomY(holdArrow, getFillTopY(holdArrow)) - ARROW_SIZE;
      } else {
        arrow->get()->setPriority(isFirst ? ARROW_LAYER_MIDDLE
                                          : ARROW_LAYER_FRONT);
        y = getFillTopY(holdArrow) + holdArrow->currentFillOffset;
      }
      arrow->setIsLastFill(isLast);
      holdArrow->currentFillOffset += ARROW_SIZE;
      break;
    }
    case ArrowType::HOLD_TAIL: {
      y = getFillBottomY(holdArrow, getFillTopY(holdArrow)) -
          HOLD_getLastFillOffset(arrow->direction) - ARROW_SIZE;
      break;
    }
    default:
      y = getYFor(arrow->timestamp);
  };

  return min(y, ARROW_INITIAL_Y);
}

bool ChartReader::isHoldActive(ArrowDirection direction) {
  return holdArrowStates[direction].isActive;
}

bool ChartReader::hasJustStopped() {
  return hasStopped && judge->isInsideTimingWindow(msecs - stopStart);
}

bool ChartReader::isAboutToResume() {
  if (!hasStopped)
    return false;

  return judge->isInsideTimingWindow((stopStart + (int)stopLength) - msecs);
}

int ChartReader::getYFor(int timestamp) {
  // arrowTime ms           -> ARROW_DISTANCE() px
  // timeLeft ms            -> x = timeLeft * ARROW_DISTANCE() / arrowTime
  int now = hasStopped ? stopStart : msecs;
  int timeLeft = timestamp - now;

  return min(ARROW_FINAL_Y() + MATH_div(timeLeft * ARROW_DISTANCE(), arrowTime),
             ARROW_INITIAL_Y);
}

void ChartReader::processNextEvents() {
  if (frameSkipCount == FRAME_SKIP) {
    frameSkipCount = 0;
  } else {
    frameSkipCount++;
    return;
  }

  processEvents(
      msecs + arrowTime, [this](EventType type, Event* event, bool* stop) {
        switch (type) {
          case EventType::NOTE:
            if (arrowPool->isFull())
              return false;

            processUniqueNote(event);
            return true;
          case EventType::HOLD_START:
            startHoldNote(event);
            return true;
          case EventType::HOLD_END:
            endHoldNote(event);
            return true;
          case EventType::SET_FAKE:
            fake = event->param;
            return true;
          default:
            if (msecs < event->timestamp)
              return false;
        }

        switch (type) {
          case EventType::SET_TEMPO: {
            u32 oldBpm = bpm;
            bpm = event->param;
            scrollBpm = event->param2;

            syncScrollSpeed();

            if (oldBpm > 0) {
              lastBpmChange = event->timestamp;
              subtick = 0;

              if (event->param3 > 0) {
                u32 arrowTimeDiff = abs((int)targetArrowTime - (int)arrowTime);
                maxArrowTimeJump = Div(arrowTimeDiff, event->param3);
              }
            } else
              syncArrowTime();
            return true;
          }
          case EventType::SET_TICKCOUNT:
            tickCount = event->param;
            lastTick = -1;
            subtick = 0;
            return true;
          case EventType::STOP:
            if (hasStopped)
              return false;

            hasStopped = true;
            stopStart = event->timestamp;
            stopLength = event->param;
            stopJudgeable = event->param2;
            return true;
          case EventType::WARP:
            if (hasStopped)
              return false;

            warpedMs += event->param;
            msecs += event->param;
            pixelBlink->blink();

            *stop = true;
            return true;
          default:
            return false;
        }
      });
}

void ChartReader::processUniqueNote(Event* event) {
  std::vector<Arrow*> arrows;

  if (GameState.mods.randomSteps) {
    u8 stompSize = STOMP_SIZE_BY_DATA[event->data >> 3] - 1;
    event->data = DATA_BY_STOMP_SIZE[stompSize][qran_range(
                      0, DATA_BY_STOMP_SIZE_COUNTS[stompSize])]
                  << 3;
  }

  forEachDirection(
      event->data, [event, &arrows, this](ArrowDirection direction) {
        arrowPool->create([event, &arrows, &direction, this](Arrow* it) {
          it->initialize(ArrowType::UNIQUE, direction, event->timestamp, fake);
          arrows.push_back(it);
        });
      });

  connectArrows(arrows);
}

void ChartReader::startHoldNote(Event* event) {
  forEachDirection(event->data, [&event, this](ArrowDirection direction) {
    holdArrowStates[direction].lastStartTime = event->timestamp;

    holdArrows->create([event, &direction, this](HoldArrow* holdArrow) {
      holdArrow->startTime = event->timestamp;
      holdArrow->endTime = 0;

      Arrow* head =
          arrowPool->create([event, &direction, &holdArrow, this](Arrow* head) {
            head->initializeHoldBorder(ArrowType::HOLD_HEAD, direction,
                                       event->timestamp, holdArrow, fake);
          });

      if (head == NULL) {
        holdArrows->discard(holdArrow->id);
        return;
      }

      holdArrow->direction = direction;
      holdArrow->fillOffsetSkip = 0;
      holdArrow->fillOffsetBottom = 0;
      holdArrow->activeFillCount = 0;
      holdArrow->lastPressTopY = HOLD_NULL;
      holdArrow->isFake = fake;
    });
  });
}

void ChartReader::endHoldNote(Event* event) {
  forEachDirection(event->data, [&event, this](ArrowDirection direction) {
    withLastHoldArrow(direction, [&event, &direction,
                                  this](HoldArrow* holdArrow) {
      holdArrow->endTime = event->timestamp;

      arrowPool->create([&holdArrow, &event, &direction, this](Arrow* tail) {
        tail->initializeHoldBorder(ArrowType::HOLD_TAIL, direction,
                                   event->timestamp, holdArrow, fake);
      });
    });
  });
}

void ChartReader::orchestrateHoldArrows() {
  holdArrows->forEachActive([this](HoldArrow* holdArrow) {
    ArrowDirection direction = holdArrow->direction;
    bool hasStarted = holdArrow->hasStarted(msecs);
    bool hasEnded = holdArrow->hasEnded(msecs);
    holdArrow->resetState();

    if (hasStarted) {
      holdArrowStates[direction].currentStartTime = holdArrow->startTime;
      holdArrowStates[direction].isActive = true;
    }

    if (hasEnded &&
        holdArrowStates[direction].currentStartTime == holdArrow->startTime)
      holdArrowStates[direction].isActive = false;

    int topY = getFillTopY(holdArrow);
    if (hasStarted && judge->isPressed(direction))
      holdArrow->updateLastPress(topY);
    int screenTopY =
        topY <= holdArrow->lastPressTopY
            ? HOLD_FILL_FINAL_Y() -
                  min(holdArrow->lastPressTopY - topY, HOLD_FILL_FINAL_Y())
            : 0;
    int bottomY = holdArrow->hasEndTime() ? getFillBottomY(holdArrow, topY)
                                          : ARROW_INITIAL_Y;

    if (bottomY < ARROW_OFFSCREEN_LIMIT) {
      holdArrows->discard(holdArrow->id);
      return;
    }

    holdArrow->fillOffsetSkip =
        max(screenTopY - topY, topY > 0 ? 0 : screenTopY);
    holdArrow->fillOffsetBottom = bottomY - topY;
    holdArrow->currentFillOffset = holdArrow->fillOffsetSkip;
    u32 fillSectionLength = holdArrow->getFillSectionLength(topY, bottomY);
    u32 targetFills = MATH_divCeil(fillSectionLength, ARROW_SIZE);

    if (hasEnded)
      return;

    while (holdArrow->activeFillCount < targetFills) {
      Arrow* fill = arrowPool->create([](Arrow* it) {});

      if (fill != NULL)
        fill->initializeHoldFill(direction, holdArrow);
      else
        break;

      holdArrow->activeFillCount++;
    }
  });
}

bool ChartReader::processTicks(int rythmMsecs, bool checkHoldArrows) {
  // 60000 ms           -> BPM beats
  // rythmMsecs ms      -> beat = msecs * BPM / 60000
  int tick =
      MATH_fracumul(rythmMsecs * bpm * tickCount, FRACUMUL_DIV_BY_MINUTE);

  bool hasChanged = tick != lastTick;

  if (hasChanged) {
    subtick++;
    if (subtick == tickCount)
      subtick = 0;

    if (checkHoldArrows) {
      u8 arrows = 0;
      bool isFake = false;
      bool canMiss = true;

      for (u32 i = 0; i < ARROWS_TOTAL; i++) {
        auto direction = static_cast<ArrowDirection>(i);

        withNextHoldArrow(direction, [&arrows, &canMiss, &direction, &isFake,
                                      this](HoldArrow* holdArrow) {
          if (holdArrow->isOccurring(msecs)) {
            arrows |= EVENT_ARROW_MASKS[direction];
            isFake = holdArrow->isFake;

            if (msecs < holdArrow->startTime + HOLD_ARROW_TICK_OFFSET_MS)
              canMiss = false;
          }
        });
      }

      if (arrows > 0 && !isFake)
        judge->onHoldTick(arrows, canMiss);
    }
  }

  lastTick = tick;

  return hasChanged && subtick == 1 && bpm > 0;
}

void ChartReader::connectArrows(std::vector<Arrow*>& arrows) {
  if (arrows.size() <= 1)
    return;

  for (u32 i = 0; i < arrows.size(); i++) {
    arrows[i]->setSiblingId(arrows[i == arrows.size() - 1 ? 0 : i + 1]->id);
  }
}

int ChartReader::getFillTopY(HoldArrow* holdArrow) {
  int firstFillOffset = HOLD_getFirstFillOffset(holdArrow->direction);

  int y = holdArrow->getHeadY(
      [holdArrow, this]() { return getYFor(holdArrow->startTime); });
  return y + firstFillOffset;
}

int ChartReader::getFillBottomY(HoldArrow* holdArrow, int topY) {
  int lastFillOffset = HOLD_getLastFillOffset(holdArrow->direction);

  return holdArrow->getTailY([holdArrow, &topY, &lastFillOffset, this]() {
    return max(getYFor(holdArrow->endTime) + lastFillOffset, topY) + ARROW_SIZE;
  });
}
