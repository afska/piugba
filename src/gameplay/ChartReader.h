#ifndef CHART_READER_H
#define CHART_READER_H

#include <functional>

#include "models/Song.h"
#include "objects/Arrow.h"
#include "utils/pool/ObjectPool.h"

class ChartReader {
 public:
  ChartReader(Chart* chart);

  bool update(u32 msecs, ObjectPool<Arrow>* arrowPool);

 private:
  Chart* chart;
  u32 eventIndex = 0;
  u32 bpm = 0;
  int lastBeat = 0;

  bool animateBpm(u32 msecs);
  void processNextEvent(u32 msecs, ObjectPool<Arrow>* arrowPool);
  void processUniqueNote(u8 data, ObjectPool<Arrow>* arrowPool);
  void processHoldNote(EventType type, u8 data, ObjectPool<Arrow>* arrowPool);
  void connectArrows(std::vector<Arrow*>& arrows);
  void forEachDirection(u8 data, std::function<void(ArrowDirection)> action);
};

#endif  // CHART_READER_H
