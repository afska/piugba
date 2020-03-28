#ifndef SPRITE_POOL_H
#define SPRITE_POOL_H

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
class ObjectQueue {
 public:
  ObjectQueue(u32 capacity, std::function<T*(u32)> create) {
    for (u32 i = 0; i < capacity; i++) {
      PooledObject<T>* pooledObject = new PooledObject<T>;
      pooledObject->object = create(i);
      objects.push_back(pooledObject);
    }

    this->capacity = capacity;
  }

  void push(std::function<void(T*)> initialize) {
    if (count == capacity)
      return;

    rear++;
    if (rear == capacity)
      rear = 0;
    auto element = objects[rear];
    element->isActive = true;
    initialize(element->object);
    count++;
  }

  void pop() {
    if (count == 0)
      return;

    auto element = objects[front];
    auto object = element->object;
    element->isActive = false;
    ((IPoolable*)object)->discard();
    front++;
    if (front == capacity)
      front = 0;
    count--;
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

  ~ObjectQueue() {
    for (auto& it : objects) {
      delete it->object;  // TODO: FIX undefined behavior
      delete it;
    }
  }

 private:
  std::vector<PooledObject<T>*> objects;
  int capacity;
  int front = 0;
  int rear = -1;
  int count = 0;
};

#endif  // SPRITE_POOL_H
