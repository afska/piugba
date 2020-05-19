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
    holdArrowsCount[i] = 0;

  targetArrowTime = ARROW_TIME[ARROW_SPEED];
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
  return min(
      arrow->getParentTimestamp() != 0
          ? getYFor(arrow->getParentTimestamp()) + arrow->getParentOffsetY()
          : getYFor(arrow->timestamp),
      ARROW_INITIAL_Y);
}

bool ChartReader::isHoldActive(ArrowDirection direction) {
  return holdArrowsCount[direction] > 0;
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
        targetArrowTime =
            Div(MINUTE, event->extra) * (MAX_ARROW_SPEED + 1 - ARROW_SPEED) * 2;

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
      Arrow* head = arrowPool->create([event, &direction](Arrow* it) {
        it->initialize(ArrowType::HOLD_HEAD, direction, event->timestamp);
      });

      if (head == NULL) {
        holdArrows->discard(holdArrow->id);
        return;
      }

      Arrow* fill = arrowPool->createWithIdGreaterThan(
          [&event, &direction, &holdArrow, this](Arrow* it) {
            it->initialize(ArrowType::HOLD_FILL, direction, holdArrow,
                           event->timestamp,
                           ARROW_SIZE - HOLD_ARROW_FILL_OFFSETS[direction]);
          },
          head->id);

      if (fill == NULL) {
        arrowPool->discard(head->id);
        holdArrows->discard(holdArrow->id);
        return;
      }

      holdArrowsCount[direction]++;
      holdArrow->direction = direction;
      holdArrow->startTime = event->timestamp;
      holdArrow->endTime = 0;
      holdArrow->fillCount = 1;
      holdArrow->lastFill = fill;
      holdArrow->tail = NULL;
    });
  });
}

void ChartReader::endHoldNote(Event* event) {
  forEachDirection(event->data, [&event, this](ArrowDirection direction) {
    withLastHoldArrow(direction, [&event, &direction,
                                  this](HoldArrow* holdArrow) {
      bool isShort = holdArrow->fillCount == 1;

      if (isShort) {
        // short (only 1 fill) -> tail's position is approximated

        arrowPool->createWithIdGreaterThan(
            [&direction, &holdArrow, this](Arrow* tail) {
              tail->initialize(ArrowType::HOLD_TAIL, direction, holdArrow,
                               holdArrow->startTime,
                               holdArrow->lastFill->getParentOffsetY() +
                                   ARROW_SIZE -
                                   HOLD_ARROW_TAIL_OFFSETS[direction]);
              holdArrow->tail = tail;
            },
            holdArrow->lastFill->id);
      } else {
        // long (extra fill) -> tail's position is perfectly accurate

        arrowPool->create([&holdArrow, &event, &direction,
                           this](Arrow* extraFill) {
          Arrow* tail = arrowPool->createWithIdGreaterThan(
              [&holdArrow, &event, &extraFill, &direction, this](Arrow* tail) {
                tail->initialize(ArrowType::HOLD_TAIL, direction,
                                 event->timestamp);
                holdArrow->tail = tail;

                extraFill->initialize(
                    ArrowType::HOLD_FILL, direction, holdArrow,
                    event->timestamp,
                    -ARROW_SIZE + HOLD_ARROW_TAIL_OFFSETS[direction]);

                // move last fill a bit up:
                holdArrow->lastFill->timestamp -= FRAME_MS;
              },
              extraFill->id);

          if (tail == NULL)
            arrowPool->discard(extraFill->id);
        });
      }

      holdArrow->endTime = event->timestamp;
    });
  });
}

void ChartReader::processHoldArrows() {
  holdArrows->forEachActive([this](HoldArrow* holdArrow) {
    if (holdArrow->endTime > 0 && msecs >= holdArrow->endTime) {
      if (holdArrow->tail != NULL)
        arrowPool->discard(holdArrow->tail->id);
      holdArrows->discard(holdArrow->id);
      holdArrowsCount[holdArrow->direction]--;
      return;
    }

    while (holdArrow->needsFillsAtTheEnd() ||
           holdArrow->needsFillsInTheMiddle()) {
      Arrow* fill = arrowPool->create([&holdArrow, this](Arrow* it) {
        it->initialize(ArrowType::HOLD_FILL, holdArrow->direction, holdArrow,
                       holdArrow->startTime,
                       holdArrow->lastFill->getParentOffsetY() + ARROW_SIZE);
        refresh(it);
        holdArrow->fillCount++;
        holdArrow->lastFill = it;
      });

      if (fill == NULL)
        break;
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
