#ifndef CHART_READER_H
#define CHART_READER_H

#include "models/Song.h"
#include "objects/Arrow.h"
#include "utils/pool/ObjectQueue.h"

class ChartReader {
 public:
  ChartReader(Chart* chart);

  void update(u32 msecs, std::unique_ptr<ObjectQueue<Arrow>> arrowQueue);

 private:
  Chart* chart;
  u32 eventIndex = 0;

  ArrowType getArrowType(u8 data);
};

#endif  // CHART_READER_H
