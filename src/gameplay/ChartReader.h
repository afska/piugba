#ifndef CHART_READER_H
#define CHART_READER_H

#include <functional>
#include <vector>

#include "models/Song.h"
#include "objects/Arrow.h"
#include "utils/pool/ObjectPool.h"

class ChartReader {
 public:
  bool hasStopped = false;

  ChartReader(Chart* chart);

  bool update(u32 msecs, ObjectPool<Arrow>* arrowPool);

 private:
  Chart* chart;
  u32 timeNeeded = 0;
  u32 eventIndex = 0;
  u32 bpm = 0;
  int lastBeat = 0;
  u32 lastBpmChange = 0;
  u32 stopStart = 0;
  u32 stopEnd = 0;
  std::array<Arrow*, ARROWS_TOTAL> holdArrows;

  bool animateBpm(u32 msecs);
  void processNextEvent(u32 msecs, ObjectPool<Arrow>* arrowPool);
  void processUniqueNote(u8 data,
                         std::vector<Arrow*>& arrows,
                         ObjectPool<Arrow>* arrowPool);
  void startHoldNote(u8 data,
                     std::vector<Arrow*>& arrows,
                     ObjectPool<Arrow>* arrowPool);
  void endHoldNote(u8 data,
                   std::vector<Arrow*>& arrows,
                   ObjectPool<Arrow>* arrowPool);
  void updateHoldArrows(u32 msecs);
  void connectArrows(std::vector<Arrow*>& arrows);
  void forEachDirection(u8 data, std::function<void(ArrowDirection)> action);
};

#endif  // CHART_READER_H
