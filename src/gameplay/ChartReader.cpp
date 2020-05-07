#include "ChartReader.h"

#include <vector>

/*
  x = x0 + v * t
  ARROW_FINAL_Y = ARROW_INITIAL_Y + ARROW_SPEED * t
  t = abs(ARROW_INITIAL_Y - ARROW_FINAL_Y) px / ARROW_SPEED px/frame
  t = (160 - 15) / 3 = (48.33 frames) * 16.73322954 ms/frame = 792,03 ms
  => Look-up table for speeds 0, 1, 2, 3 and 4 px/frame
*/
const u32 TIME_NEEDED[] = {0, 2426, 1213, 809, 607};
const u32 SNAP_THRESHOLD_MS = 20;
const int HOLD_ARROW_FILL_OFFSETS[] = {8, 5, 2, 5, 8};
const int HOLD_ARROW_TAIL_OFFSETS[] = {7, 8, 8, 8, 7};
const u32 HOLD_ARROW_POOL_SIZE = 10;
const int HOLD_ARROW_TICK_OFFSET_MS = 84;
//                                  ^ OFFSET_GOOD * msPerFrame = 5 * 16.73322954
const u32 MINUTE = 60000;
const int AUDIO_LAG = 180;

ChartReader::ChartReader(Chart* chart, Judge* judge) {
  this->chart = chart;
  this->judge = judge;
  holdArrows = std::unique_ptr<ObjectPool<HoldArrow>>{new ObjectPool<HoldArrow>(
      HOLD_ARROW_POOL_SIZE,
      [](u32 id) -> HoldArrow* { return new HoldArrow(id); })};

  timeNeeded = TIME_NEEDED[ARROW_SPEED];
};

bool ChartReader::preUpdate(int* msecs, ObjectPool<Arrow>* arrowPool) {
  int rythmMsecs = *msecs - lastBpmChange;
  bool hasChanged = animateBpm(rythmMsecs);

  *msecs = *msecs - AUDIO_LAG - (int)stoppedMs + (int)warpedMs;

  if (hasStopped)
    return hasChanged;

  processNextEvents(msecs, arrowPool);
  processHoldTicks(*msecs, rythmMsecs);

  return hasChanged;
}

void ChartReader::postUpdate(int msecs, ObjectPool<Arrow>* arrowPool) {
  if (hasStopped) {
    if (msecs >= stopStart + (int)stopLength) {
      hasStopped = false;
      stoppedMs += stopLength;
      msecs -= (int)stopLength;
    } else
      return;
  }

  predictNoteEvents(msecs, arrowPool);
  processHoldArrows(msecs, arrowPool);

  IFTIMINGTEST { logDebugInfo(msecs, arrowPool); }
};

bool ChartReader::animateBpm(int rythmMsecs) {
  // 60000 ms           -> BPM beats
  // rythmMsecs ms      -> x = millis * BPM / 60000
  int beat = Div(rythmMsecs * bpm, MINUTE);
  bool hasChanged = beat != lastBeat;
  lastBeat = beat;

  return hasChanged;
}

void ChartReader::processNextEvents(int* msecs, ObjectPool<Arrow>* arrowPool) {
  u32 currentIndex = eventIndex;
  int targetMsecs = *msecs;
  bool skipped = false;

  while (targetMsecs >= chart->events[currentIndex].timestamp &&
         currentIndex < chart->eventCount) {
    auto event = chart->events + currentIndex;
    event->index = currentIndex;
    EventType type = static_cast<EventType>((event->data & EVENT_TYPE));
    bool handled = true;

    switch (type) {
      case EventType::SET_TEMPO:
        if (bpm > 0) {
          lastBeat = -1;
          lastBpmChange = event->timestamp;
        }
        bpm = event->extra;
        break;
      case EventType::SET_TICKCOUNT:
        tickCount = event->extra;
        lastTick = -1;
        break;
      case EventType::WARP:
        warpedMs += event->extra;
        *msecs += event->extra;

        arrowPool->forEachActive([](Arrow* it) { it->scheduleDiscard(); });
        holdArrows->clear();

        eventIndex = currentIndex + 1;
        return;
      default:
        handled = false;
        skipped = true;
        break;
    }

    event->handled = handled;
    currentIndex++;
    if (!skipped)
      eventIndex++;
  }
}

void ChartReader::predictNoteEvents(int msecs, ObjectPool<Arrow>* arrowPool) {
  u32 currentIndex = eventIndex;
  int targetMsecs = msecs + timeNeeded;
  bool skipped = false;

  while (targetMsecs >= chart->events[currentIndex].timestamp &&
         currentIndex < chart->eventCount) {
    auto event = chart->events + currentIndex;
    event->index = currentIndex;
    EventType type = static_cast<EventType>((event->data & EVENT_TYPE));
    bool handled = true;

    if (event->handled) {
      currentIndex++;
      continue;
    }

    if (msecs < event->timestamp) {
      switch (type) {
        case EventType::NOTE:
          processUniqueNote(event, arrowPool);
          break;
        case EventType::HOLD_START:
          startHoldNote(event, arrowPool);
          break;
        case EventType::HOLD_END:
          endHoldNote(event, arrowPool);
          break;
        default:
          handled = false;
          skipped = true;
          break;
      }
    } else {
      switch (type) {
        case EventType::STOP:
          hasStopped = true;
          stopStart = event->timestamp;
          stopLength = event->extra;

          snapClosestArrowToHolder(msecs, arrowPool);
          event->handled = true;
          eventIndex = currentIndex + 1;
          return;
        case EventType::NOTE:
          processUniqueNote(event, arrowPool);
          break;
        case EventType::HOLD_START:
          startHoldNote(event, arrowPool);
          break;
        case EventType::HOLD_END:
          endHoldNote(event, arrowPool);
          break;
        default:
          break;
      }
    }

    event->handled = handled;
    currentIndex++;
    if (!skipped)
      eventIndex++;
  }
}

void ChartReader::processUniqueNote(Event* event,
                                    ObjectPool<Arrow>* arrowPool) {
  std::vector<Arrow*> arrows;

  forEachDirection(
      event->data, [event, &arrowPool, &arrows](ArrowDirection direction) {
        arrowPool->create([event, &arrows, &direction](Arrow* it) {
          it->initialize(ArrowType::UNIQUE, direction, event->index);
          arrows.push_back(it);
        });
      });

  connectArrows(arrows);
}

void ChartReader::startHoldNote(Event* event, ObjectPool<Arrow>* arrowPool) {
  forEachDirection(event->data, [&event, &arrowPool,
                                 this](ArrowDirection direction) {
    holdArrows->create([event, arrowPool, &direction,
                        this](HoldArrow* holdArrow) {
      Arrow* head = arrowPool->create([event, &direction](Arrow* it) {
        it->initialize(ArrowType::HOLD_HEAD, direction, event->timestamp);
      });

      if (head == NULL) {
        holdArrows->discard(holdArrow->id);
        return;
      }

      Arrow* fill = arrowPool->createWithIdGreaterThan(
          [&direction, &head](Arrow* it) {
            it->initialize(ArrowType::HOLD_FILL, direction, head->eventIndex);
            it->get()->moveTo(head->get()->getX(),
                              head->get()->getY() + ARROW_SIZE -
                                  HOLD_ARROW_FILL_OFFSETS[direction]);
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
      holdArrow->lastFill = fill;
    });
  });
}

void ChartReader::endHoldNote(Event* event, ObjectPool<Arrow>* arrowPool) {
  forEachDirection(
      event->data, [&event, &arrowPool, this](ArrowDirection direction) {
        withLastHoldArrow(direction, [&event, &arrowPool, &direction,
                                      this](HoldArrow* holdArrow) {
          auto lastFill = holdArrow->lastFill;
          int lastfillY = holdArrow->lastFill->get()->getY();

          if (lastfillY > (int)(ARROW_INITIAL_Y - ARROW_SIZE)) {
            // short (only 1 fill) -> tail's position is approximated

            arrowPool->createWithIdGreaterThan(
                [&direction, &lastFill, &lastfillY](Arrow* tail) {
                  tail->initialize(ArrowType::HOLD_TAIL, direction,
                                   lastFill->eventIndex);
                  tail->get()->moveTo(tail->get()->getX(),
                                      lastfillY + ARROW_SIZE -
                                          HOLD_ARROW_TAIL_OFFSETS[direction]);
                },
                holdArrow->lastFill->id);
          } else {
            // long (extra fill) -> tail's position is perfectly accurate

            arrowPool->create(
                [&direction, &arrowPool, &lastFill, this](Arrow* extraFill) {
                  extraFill->initialize(ArrowType::HOLD_FILL, direction,
                                        lastFill->eventIndex);

                  Arrow* tail = arrowPool->createWithIdGreaterThan(
                      [&extraFill, &direction](Arrow* tail) {
                        tail->initialize(ArrowType::HOLD_TAIL, direction,
                                         tail->eventIndex);
                        extraFill->get()->moveTo(
                            tail->get()->getX(),
                            tail->get()->getY() - ARROW_SIZE +
                                HOLD_ARROW_TAIL_OFFSETS[direction]);
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

void ChartReader::processHoldArrows(int msecs, ObjectPool<Arrow>* arrowPool) {
  holdArrows->forEachActive([&msecs, arrowPool, this](HoldArrow* holdArrow) {
    ArrowDirection direction = holdArrow->direction;

    if (holdArrow->endTime > 0 && msecs >= holdArrow->endTime) {
      holdArrows->discard(holdArrow->id);
      return;
    }

    while (holdArrow->endTime == 0 &&
           holdArrow->lastFill->get()->getY() <
               (int)(ARROW_INITIAL_Y - ARROW_SIZE + ARROW_SPEED)) {
      Arrow* fill = arrowPool->create([&direction, holdArrow, this](Arrow* it) {
        it->initialize(ArrowType::HOLD_FILL, direction,
                       holdArrow->lastFill->eventIndex);
      });

      if (fill != NULL)
        holdArrow->lastFill = fill;
    }
  });
}

void ChartReader::processHoldTicks(int msecs, int rythmMsecs) {
  int tick = Div(rythmMsecs * bpm * tickCount, MINUTE);
  bool hasChanged = tick != lastTick;

  if (hasChanged) {
    u8 arrows = 0;
    bool canMiss = true;

    for (u32 i = 0; i < ARROWS_TOTAL; i++) {
      auto direction = static_cast<ArrowDirection>(i);

      withNextHoldArrow(direction, [&msecs, &arrows, &canMiss, &direction,
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

void ChartReader::snapClosestArrowToHolder(int msecs,
                                           ObjectPool<Arrow>* arrowPool) {
  Arrow* min = NULL;
  u32 minIndex = 0;

  arrowPool->forEachActive([&msecs, &min, &minIndex, this](Arrow* itata) {
    bool isAligned = abs(msecs - chart->events[itata->eventIndex].timestamp) <
                     SNAP_THRESHOLD_MS;

    if (isAligned && (min == NULL || itata->eventIndex < minIndex)) {
      min = itata;
      minIndex = itata->eventIndex;
    }
  });

  if (min != NULL) {
    min->forAll(arrowPool, [](Arrow* arrow) {
      arrow->get()->moveTo(arrow->get()->getX(), ARROW_FINAL_Y);
    });
  }
}

// --------------------------------------------------

void ChartReader::logDebugInfo(int msecs, ObjectPool<Arrow>* arrowPool) {
  Arrow* min = NULL;
  u32 minIndex = 0;

  arrowPool->forEachActive([&min, &minIndex](Arrow* it) {
    bool isActive = it->get()->getY() >= (int)ARROW_FINAL_Y;

    if (isActive && (min == NULL || it->eventIndex < minIndex)) {
      min = it;
      minIndex = it->eventIndex;
    }
  });

  LOGN(bpm, 0);
  LOGN(msecs, 1);
  LOGN(min == NULL ? -1 : chart->events[min->eventIndex].timestamp, 2);
  LOGN(min == NULL ? -1 : min->get()->getY() - (int)ARROW_FINAL_Y, 3);
}
