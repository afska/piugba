#include "ChartReader.h"

#include <libgba-sprite-engine/gba/tonc_math.h>

#include <vector>

#include "debug/logDebugInfo.h"

const int HOLD_ARROW_FILL_OFFSETS[] = {8, 5, 2, 5, 8};
const int HOLD_ARROW_TAIL_OFFSETS[] = {7, 8, 8, 8, 7};
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

  timeNeeded = TIME_NEEDED[ARROW_SPEED];
};

bool ChartReader::preUpdate(int songMsecs) {
  int rythmMsecs = songMsecs - lastBpmChange;
  msecs = songMsecs - AUDIO_LAG - (int)stoppedMs + (int)warpedMs;

  if (hasStopped) {
    if (msecs >= stopStart + (int)stopLength) {
      hasStopped = false;
      stoppedMs += stopLength;
      msecs -= (int)stopLength;
    } else
      return false;
  }

  processNextEvents();
  processTicks(rythmMsecs);

  return subtick == 0 && bpm > 0;
}

void ChartReader::postUpdate() {
  if (hasStopped)
    return;

  predictNoteEvents();
  processHoldArrows();

  IFTIMINGTEST { logDebugInfo<CHART_DEBUG>(); }
};

int ChartReader::getYFor(int timestamp) {
  // timeNeeded ms           -> ARROW_DISTANCE px
  // timeLeft ms             -> x = timeLeft * ARROW_DISTANCE / timeNeeded
  int timeLeft = timestamp - msecs;

  return ARROW_FINAL_Y + Div(timeLeft * ARROW_DISTANCE, timeNeeded);
}

int ChartReader::getTimestampFor(int y) {
  // ARROW_DISTANCE px           -> timeNeeded ms
  // distance px                 -> x = distance * timeNeeded / ARROW_DISTANCE
  int distance = y - (int)ARROW_FINAL_Y;
  return msecs + Div(distance * timeNeeded, ARROW_DISTANCE);
}

bool ChartReader::isHoldActive(ArrowDirection direction) {
  bool isHoldActive = false;
  withNextHoldArrow(direction, [&isHoldActive, this](HoldArrow* holdArrow) {
    isHoldActive = msecs >= holdArrow->startTime &&
                   (holdArrow->endTime == 0 || msecs < holdArrow->endTime);
  });

  return isHoldActive;
}

void ChartReader::processNextEvents() {
  processEvents(msecs, [this](EventType type, Event* event, bool* stop) {
    switch (type) {
      case EventType::SET_TEMPO:
        if (bpm > 0) {
          lastBpmChange = event->timestamp;
          subtick = 0;
        }
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
  processEvents(msecs + timeNeeded,
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

                        snapClosestArrowToHolder();
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
          [&direction, &head, this](Arrow* it) {
            int y = head->get()->getY() + ARROW_SIZE -
                    HOLD_ARROW_FILL_OFFSETS[direction];
            it->initialize(ArrowType::HOLD_FILL, direction, getTimestampFor(y));
            it->get()->moveTo(head->get()->getX(), y);
          },
          head->id);

      if (fill == NULL) {
        arrowPool->discard(head->id);
        holdArrows->discard(holdArrow->id);
        return;
      }

      holdArrow->direction = direction;
      holdArrow->startTime = event->timestamp;
      holdArrow->endTime = 0;
      holdArrow->fillCount = 1;
      holdArrow->lastFill = fill;
    });
  });
}

void ChartReader::endHoldNote(Event* event) {
  forEachDirection(event->data, [&event, this](ArrowDirection direction) {
    withLastHoldArrow(
        direction, [&event, &direction, this](HoldArrow* holdArrow) {
          int lastFillY = holdArrow->lastFill->get()->getY();
          bool isShort = holdArrow->fillCount == 1;

          if (isShort) {
            // short (only 1 fill) -> tail's position is approximated

            arrowPool->createWithIdGreaterThan(
                [&direction, &lastFillY, this](Arrow* tail) {
                  int y = lastFillY + ARROW_SIZE -
                          HOLD_ARROW_TAIL_OFFSETS[direction];
                  tail->initialize(ArrowType::HOLD_TAIL, direction,
                                   getTimestampFor(y));
                  tail->get()->moveTo(tail->get()->getX(), y);
                },
                holdArrow->lastFill->id);
          } else {
            // long (extra fill) -> tail's position is perfectly accurate

            arrowPool->create([&event, &direction, this](Arrow* extraFill) {
              extraFill->initialize(ArrowType::HOLD_FILL, direction, -1);

              Arrow* tail = arrowPool->createWithIdGreaterThan(
                  [&event, &extraFill, &direction, this](Arrow* tail) {
                    tail->initialize(ArrowType::HOLD_TAIL, direction,
                                     event->timestamp);

                    int y = tail->get()->getY() - ARROW_SIZE +
                            HOLD_ARROW_TAIL_OFFSETS[direction];
                    extraFill->get()->moveTo(tail->get()->getX(), y);
                    extraFill->timestamp = getTimestampFor(y);
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
    ArrowDirection direction = holdArrow->direction;

    if (holdArrow->endTime > 0 && msecs >= holdArrow->endTime) {
      holdArrows->discard(holdArrow->id);
      return;
    }

    while (holdArrow->endTime == 0 &&
           holdArrow->lastFill->get()->getY() <
               (int)(ARROW_INITIAL_Y - ARROW_SIZE + MAX_ARROW_SPEED)) {
      Arrow* fill = arrowPool->create([&direction, this](Arrow* it) {
        it->initialize(ArrowType::HOLD_FILL, direction, msecs + timeNeeded);
      });

      if (fill != NULL) {
        holdArrow->lastFill = fill;
        holdArrow->fillCount++;
      } else
        break;
    }
  });
}

void ChartReader::processTicks(int rythmMsecs) {
  // 60000 ms           -> BPM beats
  // rythmMsecs ms      -> beat = millis * BPM / 60000
  int tick = Div(rythmMsecs * bpm * tickCount, MINUTE);
  bool hasChanged = tick != lastTick;

  if (hasChanged) {
    subtick++;
    if (subtick == tickCount)
      subtick = 0;

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

  lastTick = tick;
}

void ChartReader::connectArrows(std::vector<Arrow*>& arrows) {
  if (arrows.size() <= 1)
    return;

  for (u32 i = 0; i < arrows.size(); i++) {
    arrows[i]->setSiblingId(arrows[i == arrows.size() - 1 ? 0 : i + 1]->id);
  }
}

void ChartReader::snapClosestArrowToHolder() {
  Arrow* min = NULL;
  u32 minTimestamp = 0;

  arrowPool->forEachActive([&min, &minTimestamp, this](Arrow* it) {
    bool isAligned = it->isAligned(this);

    if (isAligned && (min == NULL || it->timestamp < minTimestamp)) {
      min = it;
      minTimestamp = it->timestamp;
    }
  });

  if (min != NULL) {
    min->forAll(arrowPool, [](Arrow* arrow) {
      arrow->get()->moveTo(arrow->get()->getX(), ARROW_FINAL_Y);
    });
  }
}
