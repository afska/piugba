#ifndef SPRITE_POOL_H
#define SPRITE_POOL_H

#include <libgba-sprite-engine/gba_engine.h>
#include <functional>

template <class T>
struct PooledObject {
 public:
  T* object;
  bool isActive = false;
};

template <class T>
class ObjectPool {
 public:
  ObjectPool(u32 size, std::function<T*()> create) {
    for (u32 i = 0; i < size; i++) {
      PooledObject<T>* pooledObject = new PooledObject<T>;
      pooledObject->object = create();
      objects.push_back(pooledObject);
    }
  }

  ~ObjectPool() {
    for (auto &it : objects) {
      free(it->object);
      free(it);
    }
  }

  void create(std::function<void(T*)> initialize) {
    for (auto &it : objects) {
      if (!it->isActive) {
        it->isActive = true;
        initialize(it->object);
        return;
      }
    }
  }

  void forEach(std::function<void(T*)> func) {
    for (auto &it : objects)
      if (it->isActive)
        func(it->object);
  }

 private:
  std::vector<PooledObject<T>*> objects;
};

#endif  // SPRITE_POOL_H
