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
const u32 MINUTE = 60000;
const u32 BEAT_UNIT = 4;
const int AUDIO_LAG = 170;

ChartReader::ChartReader(Chart* chart, Judge* judge) {
  this->chart = chart;
  this->judge = judge;
  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    holdArrows[i] = NULL;

  timeNeeded = TIME_NEEDED[ARROW_SPEED];
};

bool ChartReader::update(u32 msecs, ObjectPool<Arrow>* arrowPool) {
  processNextEvent(msecs, arrowPool);
  processHoldTicks(msecs);
  return animateBpm(msecs);
};

bool ChartReader::animateBpm(u32 msecs) {
  int msecsWithOffset = (msecs - lastBpmChange) - chart->offset;

  // 60000 ms           -> BPM beats
  // msecsWithOffset ms -> x = millis * BPM / 60000
  int beat = Div(msecsWithOffset * bpm, MINUTE);
  bool hasChanged = beat != lastBeat;
  lastBeat = beat;

  return hasChanged;
}

void ChartReader::processNextEvent(u32 msecs, ObjectPool<Arrow>* arrowPool) {
  msecs = max((int)msecs - AUDIO_LAG, 0);
  u32 currentIndex = eventIndex;
  u32 targetMsecs = msecs + timeNeeded;
  bool skipped = false;

  if (hasStopped && msecs >= stopEnd)
    hasStopped = false;

  updateHoldArrows(arrowPool);

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
          startHoldNote(event->data, arrowPool);
          break;
        case EventType::HOLD_END:
          endHoldNote(event->data, arrowPool);
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

void ChartReader::startHoldNote(u8 data, ObjectPool<Arrow>* arrowPool) {
  forEachDirection(data, [&arrowPool, this](ArrowDirection direction) {
    Arrow* head = arrowPool->create([&direction](Arrow* it) {
      it->initialize(ArrowType::HOLD_HEAD, direction);
    });

    holdArrows[direction] = arrowPool->createWithIdGreaterThan(
        [&direction, &head](Arrow* it) {
          it->initialize(ArrowType::HOLD_FILL, direction);
          it->get()->moveTo(head->get()->getX(),
                            head->get()->getY() + ARROW_HEIGHT -
                                HOLD_ARROW_FILL_OFFSETS[direction]);
        },
        head->id);
  });
}

void ChartReader::endHoldNote(u8 data, ObjectPool<Arrow>* arrowPool) {
  forEachDirection(data, [&arrowPool, this](ArrowDirection direction) {
    Arrow* fill = holdArrows[direction];
    if (fill == NULL)
      return;

    arrowPool->createWithIdGreaterThan(
        [&fill, &direction](Arrow* it) {
          it->initialize(ArrowType::HOLD_TAIL, direction);
          it->get()->moveTo(fill->get()->getX(),
                            fill->get()->getY() + ARROW_HEIGHT -
                                HOLD_ARROW_END_OFFSETS[direction]);
        },
        fill->id);

    holdArrows[direction] = NULL;
  });
}

void ChartReader::updateHoldArrows(ObjectPool<Arrow>* arrowPool) {
  for (u32 i = 0; i < ARROWS_TOTAL; i++) {
    auto direction = static_cast<ArrowDirection>(i);

    if (holdArrows[i] != NULL &&
        holdArrows[i]->get()->getY() <
            (int)(GBA_SCREEN_HEIGHT - ARROW_HEIGHT + ARROW_SPEED)) {
      Arrow* fill = arrowPool->create([&direction, this](Arrow* it) {
        it->initialize(ArrowType::HOLD_FILL, direction);
      });

      holdArrows[i] = fill;
    }
  }
}

void ChartReader::processHoldTicks(u32 msecs) {
  int msecsWithOffset = (msecs - lastBpmChange) - chart->offset;

  // TODO: Understand tickCount
  int tick = Div(msecsWithOffset * bpm * tickCount, MINUTE);
  bool hasChanged = tick != lastTick;
  if (hasChanged) {
    for (u32 i = 0; i < ARROWS_TOTAL; i++)
      if (holdArrows[i] != NULL)
        judge->onHoldTick(static_cast<ArrowDirection>(i));
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
