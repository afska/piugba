#ifndef I_POOLABLE_H
#define I_POOLABLE_H

template <class T>
class IPoolable {
  public:
    virtual void initialize(T data);
    virtual void discard();
};

#endif  // I_POOLABLE_H
