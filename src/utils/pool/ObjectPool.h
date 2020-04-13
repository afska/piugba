#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <libgba-sprite-engine/gba_engine.h>

#include <functional>
#include <vector>

#include "IPoolable.h"

inline void CRASH() {
  // *((u32*)NULL) = 1;
}

template <class T>
struct PooledObject {
 public:
  T* object;
  bool isActive = false;
};

template <class T>
class ObjectPool {
 public:
  template <typename F>
  ObjectPool(u32 size, F create) {
    for (u32 i = 0; i < size; i++) {
      PooledObject<T>* pooledObject = new PooledObject<T>;
      pooledObject->object = create(i);
      objects.push_back(pooledObject);
    }
  }

  template <typename F>
  inline T* create(F initialize) {
    for (auto& it : objects) {
      if (!it->isActive) {
        it->isActive = true;
        initialize(it->object);
        return it->object;
      }
    }

    CRASH();
    return NULL;
  }

  template <typename F>
  inline T* createWithIdGreaterThan(F initialize, u32 id) {
    for (auto& it : objects) {
      if (!it->isActive && it->object->id > id) {
        it->isActive = true;
        initialize(it->object);
        return it->object;
      }
    }

    CRASH();
    return NULL;
  }

  template <typename F>
  inline T* createWithIdLowerThan(F initialize, u32 id) {
    for (auto& it : objects) {
      if (!it->isActive && it->object->id < id) {
        it->isActive = true;
        initialize(it->object);
        return it->object;
      }
    }

    CRASH();
    return NULL;
  }

  T* getByIndex(u32 index) {
    auto element = objects[index];
    return element->isActive ? objects[index]->object : NULL;
  }

  void discard(u32 index) {
    ((IPoolable*)objects[index]->object)->discard();
    objects[index]->isActive = false;
  }

  template <typename F>
  inline void forEach(F action) {
    for (auto& it : objects)
      action(it->object);
  }

  template <typename F>
  inline void forEachActive(F action) {
    for (auto& it : objects)
      if (it->isActive)
        action(it->object);
  }

  ~ObjectPool() {
    for (auto& it : objects) {
      delete it->object;
      delete it;
    }
  }

 private:
  std::vector<PooledObject<T>*> objects;
};

#endif  // OBJECT_POOL_H
