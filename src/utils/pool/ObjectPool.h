#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <functional>
#include "IPoolable.h"

template <class T>
struct PooledObject {
 public:
  T* object;
  bool isActive = false;
};

template <class T>
class ObjectPool {
 public:
  ObjectPool(u32 size, std::function<T*(u32)> create) {
    for (u32 i = 0; i < size; i++) {
      PooledObject<T>* pooledObject = new PooledObject<T>;
      pooledObject->object = create(i);
      objects.push_back(pooledObject);
    }
  }

  T* create(std::function<void(T*)> initialize) {
    for (auto& it : objects) {
      if (!it->isActive) {
        it->isActive = true;
        initialize(it->object);
        return it->object;
      }
    }

    // (*(u8*)NULL) = 1;  // CRASH!
    return NULL;
  }

  void discard(u32 index) {
    ((IPoolable*)objects[index]->object)->discard();
    objects[index]->isActive = false;
  }

  void forEach(std::function<void(T*)> func) {
    for (auto& it : objects)
      func(it->object);
  }

  void forEachActive(std::function<void(T*)> func) {
    for (auto& it : objects)
      if (it->isActive)
        func(it->object);
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
