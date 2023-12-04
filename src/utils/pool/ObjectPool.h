#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <libgba-sprite-engine/gba_engine.h>

#include "IPoolable.h"

const u32 MAX_OBJECT_POOL_SIZE = 50;

static bool isWarningVisible = false;
inline void LOG_WARNING() {
  if (!isWarningVisible) {
    LOGSTR("!", 0);
    isWarningVisible = true;
  }
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
    this->size = size;
    for (u32 i = 0; i < size; i++) {
      objects[i].object = create(i);
      activeIndices[i] = -1;
    }
  }

  template <typename F>
  inline T* create(F initialize) {
    for (u32 i = 0; i < size; i++) {
      auto& it = objects[i];
      if (!it.isActive) {
        it.isActive = true;
        activeIndices[activeObjects] = i;
        activeObjects++;
        initialize(it.object);
        return it.object;
      }
    }

#ifdef SENV_DEBUG
    LOG_WARNING();
#endif

    return NULL;
  }

  inline T* getByIndex(u32 index) {
    auto element = objects[index];
    return element.isActive ? objects[index].object : NULL;
  }

  inline bool isFull() { return activeObjects == size; }
  u32 getActiveObjects() { return activeObjects; }

  void discard(u32 index) {
    for (u32 i = 0; i < activeObjects; i++) {
      if (activeIndices[i] == index) {
        activeIndices[i] = activeIndices[activeObjects - 1];
        activeIndices[activeObjects - 1] = -1;
        activeObjects--;

        ((IPoolable*)objects[index].object)->discard();
        objects[index].isActive = false;
        return;
      }
    }
  }

  void clear() {
    for (u32 i = 0; i < size; i++)
      if (objects[i].isActive)
        discard(i);
  }

  template <typename F>
  inline void forEach(F action) {
    for (u32 i = 0; i < size; i++)
      action(objects[i].object);
  }

  template <typename F>
  inline void forEachActive(F action) {
    for (int i = activeObjects - 1; i >= 0; i--) {
      u32 currentIndex = activeIndices[i];
      if (objects[currentIndex].isActive)
        action(objects[currentIndex].object);
    }
  }

  ~ObjectPool() {
    for (u32 i = 0; i < size; i++)
      delete objects[i].object;
  }

 private:
  u32 size;
  std::array<PooledObject<T>, MAX_OBJECT_POOL_SIZE> objects;
  std::array<u32, MAX_OBJECT_POOL_SIZE> activeIndices;
  u32 activeObjects = 0;
};

#endif  // OBJECT_POOL_H
