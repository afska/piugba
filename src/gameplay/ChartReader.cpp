#include "ChartReader.h"

#include <libgba-sprite-engine/gba/tonc_math.h>

#include <vector>

#include "debug/logDebugInfo.h"
#include "utils/MathUtils.h"

const u32 HOLD_ARROW_POOL_SIZE = 10;

ChartReader::ChartReader(Chart* chart,
                         ObjectPool<Arrow>* arrowPool,
                         Judge* judge) {
  this->chart = chart;
  this->arrowPool = arrowPool;
  this->judge = judge;

  holdArrows = std::unique_ptr<ObjectPool<HoldArrow>>{new ObjectPool<HoldArrow>(
      HOLD_ARROW_POOL_SIZE,
      [](u32 id) -> HoldArrow* { return new HoldArrow(id); })};
  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    holdArrowFlags[i] = false;

  multiplier = ARROW_DEFAULT_MULTIPLIER;
  targetArrowTime = ARROW_TIME[multiplier];
  syncArrowTime();
};

bool ChartReader::preUpdate(int songMsecs) {
  int rythmMsecs = songMsecs - lastBpmChange;
  msecs = songMsecs - AUDIO_LAG - (int)stoppedMs + (int)warpedMs;

  if (targetArrowTime > arrowTime)
    arrowTime += min(targetArrowTime - arrowTime, MAX_ARROW_TIME_JUMP);
  else
    arrowTime -= min(arrowTime - targetArrowTime, MAX_ARROW_TIME_JUMP);

  if (hasStopped) {
    if (msecs >= stopStart + (int)stopLength) {
      hasStopped = false;
      stoppedMs += stopLength;
      msecs -= (int)stopLength;
    } else
      return processTicks(rythmMsecs, false);
  }

  holdArrows->forEachActive([](HoldArrow* it) { it->resetState(); });
  processNextEvents();
  return processTicks(rythmMsecs, true);
}

void ChartReader::postUpdate() {
  if (hasStopped)
    return;

  predictNoteEvents();
  processHoldArrows();

  IFTIMINGTEST { logDebugInfo<CHART_DEBUG>(); }
};

int ChartReader::getYFor(Arrow* arrow) {
  HoldArrow* holdArrow = arrow->getHoldArrow();

  int y;
  switch (arrow->type) {
    {
      case ArrowType::HOLD_HEAD_EXTRA_FILL:
      case ArrowType::HOLD_HEAD_ARROW:
        int offsetY = ARROW_SIZE - HOLD_ARROW_FILL_OFFSETS[arrow->direction];
        y = getHeadY(arrow);
        if (arrow->type == ArrowType::HOLD_HEAD_ARROW)
          y -= offsetY;
        break;
    }
    {
      case ArrowType::HOLD_TAIL_EXTRA_FILL:
      case ArrowType::HOLD_TAIL_ARROW:
        int headY = getHeadY(arrow);
        int offsetY = -ARROW_SIZE + HOLD_ARROW_TAIL_OFFSETS[arrow->direction];
        y = getTailY(arrow, headY, offsetY);
        if (arrow->type == ArrowType::HOLD_TAIL_ARROW)
          y -= offsetY;
        break;
    }
    {
      case ArrowType::HOLD_FILL:
        if (holdArrow != NULL) {
          if (holdArrow->currentFillIndex == holdArrow->fillCount - 1) {
            y = ARROW_OFFSCREEN_LIMIT - 1;
            break;
          }

          int headY = getHeadY(arrow);
          int fillIndex = holdArrow->currentFillIndex + holdArrow->fillSkip;
          int offsetY = ARROW_SIZE * fillIndex;
          y = headY + offsetY;
          holdArrow->currentFillIndex++;
        } else
          y = ARROW_OFFSCREEN_LIMIT - 1;  // TODO: Revise
        break;
    }
    default:
      y = getYFor(arrow->timestamp);
  };

  return min(y, ARROW_INITIAL_Y);
}

bool ChartReader::isHoldActive(ArrowDirection direction) {
  return holdArrowFlags[direction];
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
  // arrowTime ms           -> ARROW_DISTANCE px
  // timeLeft ms             -> x = timeLeft * ARROW_DISTANCE / arrowTime
  int now = hasStopped ? stopStart : msecs;
  int timeLeft = timestamp - now;

  return min(ARROW_FINAL_Y + Div(timeLeft * ARROW_DISTANCE, arrowTime),
             ARROW_INITIAL_Y);
}

void ChartReader::processNextEvents() {
  processEvents(msecs, [this](EventType type, Event* event, bool* stop) {
    switch (type) {
      case EventType::SET_TEMPO:
        setScrollSpeed(event->extra);

        if (bpm > 0) {
          lastBpmChange = event->timestamp;
          subtick = 0;
        } else
          syncArrowTime();

        bpm = event->extra;
        return true;
      case EventType::SET_TICKCOUNT:
        tickCount = event->extra;
        lastTick = -1;
        subtick = 0;
        return true;
      case EventType::WARP:
        warpedMs += event->extra;
        msecs += event->extra;

        arrowPool->forEachActive([](Arrow* it) { it->scheduleDiscard(); });
        holdArrows->clear();

        *stop = true;
        return true;
      default:
        return false;
    }
  });
}

void ChartReader::predictNoteEvents() {
  processEvents(msecs + targetArrowTime,
                [this](EventType type, Event* event, bool* stop) {
                  if (msecs < event->timestamp) {
                    switch (type) {
                      case EventType::NOTE:
                        processUniqueNote(event);
                        return true;
                      case EventType::HOLD_START:
                        startHoldNote(event);
                        return true;
                      case EventType::HOLD_END:
                        endHoldNote(event);
                        return true;
                      default:
                        return false;
                    }
                  } else {
                    switch (type) {
                      case EventType::NOTE:
                        processUniqueNote(event);
                        return true;
                      case EventType::HOLD_START:
                        startHoldNote(event);
                        return true;
                      case EventType::HOLD_END:
                        endHoldNote(event);
                        return true;
                      case EventType::STOP:
                        hasStopped = true;
                        stopStart = event->timestamp;
                        stopLength = event->extra;
                        *stop = true;
                        return true;
                      default:
                        return false;
                    }
                  }
                });
}

void ChartReader::processUniqueNote(Event* event) {
  std::vector<Arrow*> arrows;

  forEachDirection(
      event->data, [event, &arrows, this](ArrowDirection direction) {
        arrowPool->create([event, &arrows, &direction](Arrow* it) {
          it->initialize(ArrowType::UNIQUE, direction, event->timestamp);
          arrows.push_back(it);
        });
      });

  connectArrows(arrows);
}

void ChartReader::startHoldNote(Event* event) {
  forEachDirection(event->data, [&event, this](ArrowDirection direction) {
    holdArrows->create([event, &direction, this](HoldArrow* holdArrow) {
      holdArrow->startTime = event->timestamp;
      holdArrow->endTime = 0;

      Arrow* head =
          arrowPool->create([event, &direction, &holdArrow, this](Arrow* head) {
            head->initializeHoldBorder(ArrowType::HOLD_HEAD_ARROW, direction,
                                       event->timestamp, holdArrow);
            arrowPool->createWithIdGreaterThan(
                [event, &direction, &holdArrow](Arrow* extraFill) {
                  extraFill->initializeHoldBorder(
                      ArrowType::HOLD_HEAD_EXTRA_FILL, direction,
                      event->timestamp, holdArrow);
                },
                head->id);
          });

      if (head == NULL) {
        holdArrows->discard(holdArrow->id);
        return;
      }

      holdArrow->direction = direction;
      holdArrow->fillSkip = 0;
      holdArrow->fillCount = 0;
      holdArrow->activeFillCount = 0;
    });
  });
}

void ChartReader::endHoldNote(Event* event) {
  forEachDirection(event->data, [&event, this](ArrowDirection direction) {
    withLastHoldArrow(direction, [&event, &direction,
                                  this](HoldArrow* holdArrow) {
      holdArrow->endTime = event->timestamp;

      arrowPool->create([&holdArrow, &event, &direction,
                         this](Arrow* extraFill) {
        Arrow* tail = arrowPool->createWithIdGreaterThan(
            [&holdArrow, &event, &extraFill, &direction, this](Arrow* tail) {
              extraFill->initializeHoldBorder(ArrowType::HOLD_TAIL_EXTRA_FILL,
                                              direction, event->timestamp,
                                              holdArrow);
              tail->initializeHoldBorder(ArrowType::HOLD_TAIL_ARROW, direction,
                                         event->timestamp, holdArrow);
            },
            extraFill->id);

        if (tail == NULL)
          arrowPool->discard(extraFill->id);
      });
    });
  });
}

void ChartReader::processHoldArrows() {
  holdArrows->forEachActive([this](HoldArrow* holdArrow) {
    if (msecs >= holdArrow->startTime)
      holdArrowFlags[holdArrow->direction] = true;

    if (holdArrow->endTime > 0 && msecs >= holdArrow->endTime) {
      holdArrows->discard(holdArrow->id);
      holdArrowFlags[holdArrow->direction] = false;
      return;
    }

    holdArrow->fillSkip = calculateFills(holdArrow, -ARROW_SIZE);
    holdArrow->fillCount =
        1 + calculateFills(holdArrow, holdArrow->cachedEndY != HOLD_CACHE_MISS
                                          ? holdArrow->cachedEndY
                                          : ARROW_INITIAL_Y);
    while (holdArrow->activeFillCount < holdArrow->getTargetFills()) {
      Arrow* fill = arrowPool->create([](Arrow* it) {});

      if (fill != NULL)
        fill->initializeHoldFill(holdArrow->direction, holdArrow);
      else
        break;

      holdArrow->activeFillCount++;
    }

    // LOGSTR("skip: " + std::to_string(holdArrow->fillSkip), 0);
    // LOGSTR("count: " + std::to_string(holdArrow->fillCount), 1);
    // u32 activeSprites = 0;
    // arrowPool->forEachActive([&activeSprites](Arrow* it) { activeSprites++;
    // }); LOGSTR("sprites: " + std::to_string(activeSprites), 2);
    // LOGN(holdArrow->cachedStartY, 3);
    // // TODO: REMOVE
  });
}

bool ChartReader::processTicks(int rythmMsecs, bool checkHoldArrows) {
  // 60000 ms           -> BPM beats
  // rythmMsecs ms      -> beat = millis * BPM / 60000
  int tick = Div(rythmMsecs * bpm * tickCount, MINUTE);
  bool hasChanged = tick != lastTick;

  if (hasChanged) {
    subtick++;
    if (subtick == tickCount)
      subtick = 0;

    if (checkHoldArrows) {
      u8 arrows = 0;
      bool canMiss = true;

      for (u32 i = 0; i < ARROWS_TOTAL; i++) {
        auto direction = static_cast<ArrowDirection>(i);

        withNextHoldArrow(direction, [&arrows, &canMiss, &direction,
                                      this](HoldArrow* holdArrow) {
          if (msecs >= holdArrow->startTime) {
            arrows |= EVENT_ARROW_MASKS[direction];

            if (msecs < holdArrow->startTime + HOLD_ARROW_TICK_OFFSET_MS)
              canMiss = false;
          }
        });
      }

      if (arrows > 0)
        judge->onHoldTick(arrows, canMiss);
    }
  }

  lastTick = tick;

  return hasChanged && subtick == 0 && bpm > 0;
}

void ChartReader::connectArrows(std::vector<Arrow*>& arrows) {
  if (arrows.size() <= 1)
    return;

  for (u32 i = 0; i < arrows.size(); i++) {
    arrows[i]->setSiblingId(arrows[i == arrows.size() - 1 ? 0 : i + 1]->id);
  }
}

int ChartReader::getHeadY(Arrow* arrow) {
  HoldArrow* holdArrow = arrow->getHoldArrow();
  int offsetY = ARROW_SIZE - HOLD_ARROW_FILL_OFFSETS[arrow->direction];

  int y = holdArrow != NULL
              ? (((((((((((((holdArrow->getStartY([arrow, this]() {
                  return getYFor(arrow->getHoldStartTime());
                }))))))))))))))
              : getYFor(arrow->getHoldStartTime());
  return y + offsetY + ARROW_SIZE;
}

int ChartReader::getTailY(Arrow* arrow, int headY, int offsetY) {
  HoldArrow* holdArrow = arrow->getHoldArrow();

  return holdArrow != NULL
             ? holdArrow->getEndY([arrow, headY, offsetY, this]() {
                 return max(getYFor(arrow->getHoldEndTime()) + offsetY, headY);
               })
             : max(getYFor(arrow->getHoldEndTime()) + offsetY, headY);
}

int ChartReader::calculateFills(HoldArrow* holdArrow, int endY) {
  int minimumDistance =
      ARROW_SIZE * 2 - HOLD_ARROW_FILL_OFFSETS[holdArrow->direction];

  int startY = holdArrow->cachedStartY != HOLD_CACHE_MISS
                   ? holdArrow->cachedStartY
                   : getYFor(holdArrow->startTime);
  int distance = endY - startY;
  return max(MATH_divCeil(distance - minimumDistance, ARROW_SIZE), 0);
}
