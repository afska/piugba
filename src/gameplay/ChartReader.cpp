#include "ChartReader.h"

#include <libgba-sprite-engine/gba/tonc_math.h>

#include <vector>

#include "debug/logDebugInfo.h"

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
  int y;
  switch (arrow->type) {
    {
      case ArrowType::HOLD_HEAD:
        y = getYFor(arrow->getHoldStartTime());
        break;
    }
    {
      case ArrowType::HOLD_FILL:
        int headY = getYFor(arrow->getHoldStartTime());  // TODO: Cache values
                                                         // in the same frame
        int fillIndex = arrow->getFillIndex();
        int offset0Y = ARROW_SIZE - HOLD_ARROW_FILL_OFFSETS[arrow->direction];
        int offsetY = offset0Y + ARROW_SIZE * fillIndex;
        y = max(headY + offsetY, headY + offset0Y);
        break;
    }
    {
      case ArrowType::HOLD_TAIL_EXTRA_FILL:
      case ArrowType::HOLD_TAIL_ARROW:
        int headY = getYFor(arrow->getHoldStartTime());
        int offset0Y = ARROW_SIZE - HOLD_ARROW_FILL_OFFSETS[arrow->direction];
        int offsetY = -ARROW_SIZE + HOLD_ARROW_TAIL_OFFSETS[arrow->direction];
        y = max(getYFor(arrow->getHoldEndTime()) + offsetY, headY + offset0Y);
        if (arrow->type == ArrowType::HOLD_TAIL_ARROW)
          y -= offsetY;
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
          arrowPool->create([event, &direction, &holdArrow](Arrow* it) {
            it->initializeHoldBorder(ArrowType::HOLD_HEAD, direction,
                                     event->timestamp, holdArrow);
          });

      if (head == NULL) {
        holdArrows->discard(holdArrow->id);
        return;
      }

      holdArrow->direction = direction;
      holdArrow->fillCount = 0;
      holdArrow->lastFill = head;
      holdArrow->tail = NULL;
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
              holdArrow->tail = extraFill;
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

    // TODO: Calculate needed fillCount based on current speed and remove/add to
    // match it

    while (holdArrow->needsFillsAtTheEnd() ||
           holdArrow->needsFillsInTheMiddle()) {
      Arrow* fill = holdArrow->fillCount == 0
                        ? arrowPool->createWithIdGreaterThan(
                              [](Arrow* it) {}, holdArrow->lastFill->id)
                        : arrowPool->create([](Arrow* it) {});

      if (fill != NULL)
        fill->initializeHoldFill(holdArrow->direction, holdArrow,
                                 holdArrow->fillCount);
      else
        break;

      refresh(fill);
      holdArrow->fillCount++;
      holdArrow->lastFill = fill;
    }
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

void ChartReader::refresh(Arrow* arrow) {
  arrow->get()->moveTo(arrow->get()->getX(), getYFor(arrow));
}
