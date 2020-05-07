#ifndef TIMING_PROVIDER_H
#define TIMING_PROVIDER_H

class TimingProvider {
 public:
  virtual int getMsecs() = 0;
  virtual bool isStopped() = 0;
};

#endif  // TIMING_PROVIDER_H
