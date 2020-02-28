#ifndef SPRITE_POOL_H
#define SPRITE_POOL_H

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

  void create(std::function<void(T*)> initialize) {
    for (auto& it : objects) {
      if (!it->isActive) {
        it->isActive = true;
        initialize(it->object);
        return;
      }
    }
  }

  void discard(u32 index) {
    objects[index]->isActive = false;
  }

  void forEach(std::function<void(T*)> func) {
    for (auto& it : objects)
      if (it->isActive)
        func(it->object);
  }

  ~ObjectPool() {
    for (auto& it : objects) {
      free(it->object);
      free(it);
    }
  }

 private:
  std::vector<PooledObject<T>*> objects;
};

#endif  // SPRITE_POOL_H
