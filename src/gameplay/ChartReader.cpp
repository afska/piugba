#include "ChartReader.h"

#include <libgba-sprite-engine/gba/tonc_math.h>
#include <libgba-sprite-engine/gba_engine.h>

#include <vector>

/*
  x = x0 + v * t
  ARROW_CORNER_MARGIN_Y = GBA_SCREEN_HEIGHT + ARROW_SPEED * t
  t = (ARROW_CORNER_MARGIN_Y - GBA_SCREEN_HEIGHT) px / ARROW_SPEED px/frame
  t = (15 - 160) / 3 = -48.33 frames * 16.73322954 ms/frame = -808,77 frames
  => Look-up table for speeds 0, 1, 2, 3 and 4 px/frame
*/
const u32 TIME_NEEDED[] = {0, 2426, 1213, 809, 607};
const int HOLD_ARROW_FILL_OFFSETS[] = {8, 5, 2, 5, 8};
const int HOLD_ARROW_END_OFFSETS[] = {7, 8, 8, 8, 7};
const u32 HOLD_ARROW_POOL_SIZE = 4;
const u32 MINUTE = 60000;
const u32 BEAT_UNIT = 4;
const int AUDIO_LAG = 170;

ChartReader::ChartReader(Chart* chart, Judge* judge) {
  this->chart = chart;
  this->judge = judge;
  holdArrows = std::unique_ptr<ObjectPool<HoldArrow>>{new ObjectPool<HoldArrow>(
      HOLD_ARROW_POOL_SIZE,
      [](u32 id) -> HoldArrow* { return new HoldArrow(id); })};

  timeNeeded = TIME_NEEDED[ARROW_SPEED];
};

bool ChartReader::update(u32* msecs, ObjectPool<Arrow>* arrowPool) {
  int msecsWithOffset = (*msecs - lastBpmChange) - chart->offset;
  bool hasChanged = animateBpm(msecsWithOffset);
  *msecs = max((int)*msecs - AUDIO_LAG, 0);

  processNextEvent(*msecs, arrowPool);
  processHoldArrows(*msecs, arrowPool);
  processHoldTicks(*msecs, msecsWithOffset);

  return hasChanged;
};

void ChartReader::withNextHoldArrow(ArrowDirection direction,
                                    std::function<void(HoldArrow*)> action) {
  holdArrows->forEachActiveWithBreak(
      [&direction, &action, this](HoldArrow* holdArrow) {
        if (holdArrow->direction != direction)
          return true;

        action(holdArrow);

        return false;
      });
}

bool ChartReader::animateBpm(int msecsWithOffset) {
  // 60000 ms           -> BPM beats
  // msecsWithOffset ms -> x = millis * BPM / 60000
  int beat = Div(msecsWithOffset * bpm, MINUTE);
  bool hasChanged = beat != lastBeat;
  lastBeat = beat;

  return hasChanged;
}

void ChartReader::processNextEvent(u32 msecs, ObjectPool<Arrow>* arrowPool) {
  u32 currentIndex = eventIndex;
  u32 targetMsecs = msecs + timeNeeded;
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
            lastBpmChange = msecs;
          }
          bpm = event->extra;
          break;
        case EventType::SET_TICKCOUNT:
          tickCount = event->extra;
          lastTick = -1;
          break;
        case EventType::STOP:
          hasStopped = true;
          stopStart = msecs;
          stopEnd = msecs + event->extra;
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
    arrows.push_back(arrowPool->create([&direction](Arrow* it) {
      it->initialize(ArrowType::UNIQUE, direction);
    }));
  });

  connectArrows(arrows);
}

void ChartReader::startHoldNote(Event* event, ObjectPool<Arrow>* arrowPool) {
  forEachDirection(
      event->data, [&event, &arrowPool, this](ArrowDirection direction) {
        Arrow* head = arrowPool->create([&direction](Arrow* it) {
          it->initialize(ArrowType::HOLD_HEAD, direction);
        });

        holdArrows->create(
            [event, arrowPool, &direction, &head](HoldArrow* holdArrow) {
              holdArrow->direction = direction;
              holdArrow->startTime = event->timestamp;
              holdArrow->endTime = 0;
              holdArrow->lastFill = arrowPool->createWithIdGreaterThan(
                  [&direction, &head](Arrow* it) {
                    it->initialize(ArrowType::HOLD_FILL, direction);
                    it->get()->moveTo(head->get()->getX(),
                                      head->get()->getY() + ARROW_HEIGHT -
                                          HOLD_ARROW_FILL_OFFSETS[direction]);
                  },
                  head->id);
            });
      });
}

void ChartReader::endHoldNote(Event* event, ObjectPool<Arrow>* arrowPool) {
  forEachDirection(
      event->data, [&event, &arrowPool, this](ArrowDirection direction) {
        withNextHoldArrow(
            direction, [&event, &arrowPool, &direction](HoldArrow* holdArrow) {
              Arrow* fill = holdArrow->lastFill;
              arrowPool->createWithIdGreaterThan(
                  [&fill, &direction](Arrow* it) {
                    it->initialize(ArrowType::HOLD_TAIL, direction);
                    it->get()->moveTo(fill->get()->getX(),
                                      fill->get()->getY() + ARROW_HEIGHT -
                                          HOLD_ARROW_END_OFFSETS[direction]);
                  },
                  fill->id);
              holdArrow->endTime = event->timestamp;
            });
      });
}

void ChartReader::processHoldArrows(u32 msecs, ObjectPool<Arrow>* arrowPool) {
  holdArrows->forEachActive([&msecs, arrowPool, this](HoldArrow* holdArrow) {
    ArrowDirection direction = holdArrow->direction;

    if (holdArrow->endTime > 0 && msecs >= holdArrow->endTime) {
      holdArrows->discard(holdArrow->id);
      return;
    }

    if (holdArrow->endTime == 0 &&
        holdArrow->lastFill->get()->getY() <
            (int)(GBA_SCREEN_HEIGHT - ARROW_HEIGHT + ARROW_SPEED)) {
      Arrow* fill = arrowPool->create([&direction, this](Arrow* it) {
        it->initialize(ArrowType::HOLD_FILL, direction);
      });

      holdArrow->lastFill = fill;
    }

    return;
  });
}

void ChartReader::processHoldTicks(u32 msecs, int msecsWithOffset) {
  // TODO: Understand tickCount
  int tick = Div(msecsWithOffset * bpm * tickCount, MINUTE);
  bool hasChanged = tick != lastTick;

  if (hasChanged) {
    for (u32 i = 0; i < ARROWS_TOTAL; i++) {
      auto direction = static_cast<ArrowDirection>(i);

      withNextHoldArrow(direction,
                        [&msecs, &direction, this](HoldArrow* holdArrow) {
                          if (msecs >= holdArrow->startTime)
                            judge->onHoldTick(direction);
                        });
    }
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

void ChartReader::forEachDirection(u8 data,
                                   std::function<void(ArrowDirection)> action) {
  if (data & EVENT_ARROW_DOWNLEFT)
    action(ArrowDirection::DOWNLEFT);

  if (data & EVENT_ARROW_UPLEFT)
    action(ArrowDirection::UPLEFT);

  if (data & EVENT_ARROW_CENTER)
    action(ArrowDirection::CENTER);

  if (data & EVENT_ARROW_UPRIGHT)
    action(ArrowDirection::UPRIGHT);

  if (data & EVENT_ARROW_DOWNRIGHT)
    action(ArrowDirection::DOWNRIGHT);
}
