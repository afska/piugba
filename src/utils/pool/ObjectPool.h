#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <libgba-sprite-engine/gba_engine.h>

#include <vector>

#include "IPoolable.h"

inline void LOGERROR() {
  IFTEST { LOGSTR("no-memory!", 0); }
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
        activeObjects++;
        initialize(it->object);
        return it->object;
      }
    }

    LOGERROR();
    return NULL;
  }

  T* getByIndex(u32 index) {
    auto element = objects[index];
    return element->isActive ? objects[index]->object : NULL;
  }

  bool isFull() { return activeObjects == objects.size(); }

  void discard(u32 index) {
    ((IPoolable*)objects[index]->object)->discard();
    objects[index]->isActive = false;
    activeObjects--;
  }

  void clear() {
    for (u32 i = 0; i < objects.size(); i++)
      discard(i);
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
  u32 activeObjects = 0;
};

#endif  // OBJECT_POOL_H
