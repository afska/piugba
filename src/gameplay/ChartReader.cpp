#include "ChartReader.h"

#include <libgba-sprite-engine/gba/tonc_math.h>
#include <libgba-sprite-engine/gba_engine.h>

#include <vector>

/*
  x = x0 + v * t
  ARROW_CORNER_MARGIN_Y = GBA_SCREEN_HEIGHT + ARROW_SPEED * t
  t = abs(GBA_SCREEN_HEIGHT - ARROW_CORNER_MARGIN_Y) px / ARROW_SPEED px/frame
  t = (160 - 15) / 3 = (48.33 frames - 1) * 16.73322954 ms/frame = 792,03 ms
  (we substract 1 because arrows start moving the same tick they're created)
  => Look-up table for speeds 0, 1, 2, 3 and 4 px/frame
*/
const u32 TIME_NEEDED[] = {0, 2410, 1196, 792, 590};
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

bool ChartReader::update(int* msecs, ObjectPool<Arrow>* arrowPool) {
  int msecsWithOffset = (*msecs - lastBpmChange) - chart->offset;
  bool hasChanged = animateBpm(msecsWithOffset);
  *msecs -= AUDIO_LAG;

  processNextEvent(*msecs, arrowPool);
  processHoldArrows(*msecs, arrowPool);
  processHoldTicks(*msecs, msecsWithOffset);

  return hasChanged;
};

bool ChartReader::animateBpm(int msecsWithOffset) {
  // 60000 ms           -> BPM beats
  // msecsWithOffset ms -> x = millis * BPM / 60000
  int beat = Div(msecsWithOffset * bpm, MINUTE);
  bool hasChanged = beat != lastBeat;
  lastBeat = beat;

  return hasChanged;
}

void ChartReader::processNextEvent(int msecs, ObjectPool<Arrow>* arrowPool) {
  u32 currentIndex = eventIndex;
  int targetMsecs = msecs + timeNeeded;
  bool skipped = false;

  if (hasStopped && msecs >= stopEnd)
    hasStopped = false;

  while (targetMsecs >= chart->events[currentIndex].timestamp &&
         currentIndex < chart->eventCount) {
    auto event = chart->events + currentIndex;
    EventType type = static_cast<EventType>((event->data & EVENT_TYPE));
    bool handled = true;

    if (event->handled) {
      currentIndex++;
      continue;
    }

    if (type == EventType::STOP)
      targetMsecs += event->extra;

    if (msecs < event->timestamp) {
      // events with anticipation

      switch (type) {
        case EventType::NOTE:
          processUniqueNote(event->data, arrowPool);
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
      // exact events

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
        case EventType::STOP:
          hasStopped = true;
          stopEnd = msecs + (int)event->extra;
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

void ChartReader::processUniqueNote(u8 data, ObjectPool<Arrow>* arrowPool) {
  std::vector<Arrow*> arrows;

  forEachDirection(data, [&arrowPool, &arrows](ArrowDirection direction) {
    arrowPool->create([&arrows, &direction](Arrow* it) {
      it->initialize(ArrowType::UNIQUE, direction);
      arrows.push_back(it);
    });
  });

  connectArrows(arrows);
}

void ChartReader::startHoldNote(Event* event, ObjectPool<Arrow>* arrowPool) {
  forEachDirection(
      event->data, [&event, &arrowPool, this](ArrowDirection direction) {
        holdArrows->create(
            [event, arrowPool, &direction, this](HoldArrow* holdArrow) {
              Arrow* head = arrowPool->create([&direction](Arrow* it) {
                it->initialize(ArrowType::HOLD_HEAD, direction);
              });

              if (head == NULL) {
                holdArrows->discard(holdArrow->id);
                return;
              }

              Arrow* fill = arrowPool->createWithIdGreaterThan(
                  [&direction, &head](Arrow* it) {
                    it->initialize(ArrowType::HOLD_FILL, direction);
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
          int lastfillY = holdArrow->lastFill->get()->getY();

          if (lastfillY > GBA_SCREEN_HEIGHT - (int)ARROW_SIZE) {
            // short (only 1 fill) -> tail's position is approximated

            arrowPool->createWithIdGreaterThan(
                [&direction, &lastfillY](Arrow* tail) {
                  tail->initialize(ArrowType::HOLD_TAIL, direction);
                  tail->get()->moveTo(tail->get()->getX(),
                                      lastfillY + ARROW_SIZE -
                                          HOLD_ARROW_TAIL_OFFSETS[direction]);
                },
                holdArrow->lastFill->id);
          } else {
            // long (extra fill) -> tail's position is perfectly accurate

            arrowPool->create([&direction, &arrowPool, this](Arrow* extraFill) {
              extraFill->initialize(ArrowType::HOLD_FILL, direction);

              Arrow* tail = arrowPool->createWithIdGreaterThan(
                  [&extraFill, &direction](Arrow* tail) {
                    tail->initialize(ArrowType::HOLD_TAIL, direction);
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

    if (holdArrow->endTime == 0 &&
        holdArrow->lastFill->get()->getY() <
            (int)(GBA_SCREEN_HEIGHT - ARROW_SIZE + ARROW_SPEED)) {
      Arrow* fill = arrowPool->create([&direction, this](Arrow* it) {
        it->initialize(ArrowType::HOLD_FILL, direction);
      });

      if (fill != NULL)
        holdArrow->lastFill = fill;
    }
  });
}

void ChartReader::processHoldTicks(int msecs, int msecsWithOffset) {
  int tick = Div(msecsWithOffset * bpm * tickCount, MINUTE);
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
