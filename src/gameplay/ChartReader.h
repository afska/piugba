#ifndef CHART_READER_H
#define CHART_READER_H

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
  void processNote(u8 data, ObjectPool<Arrow>* arrowPool);
};

#endif  // CHART_READER_H
