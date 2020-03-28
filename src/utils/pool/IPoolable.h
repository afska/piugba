#ifndef I_POOLABLE_H
#define I_POOLABLE_H

class IPoolable {
 public:
  virtual void discard();

  virtual ~IPoolable() {}
};

#endif  // I_POOLABLE_H
