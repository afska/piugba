#ifndef SONG_SCENE_H
#define SONG_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>
#include "objects/Arrow.h"
#include "objects/ArrowHolder.h"
#include "objects/Combo.h"
#include "objects/ComboDigit.h"
#include "objects/DanceAnimation.h"
#include "objects/Feedback.h"
#include "utils/pool/ObjectQueue.h"

class SongScene : public Scene {
 public:
  SongScene(std::shared_ptr<GBAEngine> engine) : Scene(engine) {}

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;
  void setMsecs(u32 msecs);

  void load() override;
  void tick(u16 keys) override;

 private:
  std::unique_ptr<Background> bg;
  std::unique_ptr<DanceAnimation> animation;
  std::vector<std::unique_ptr<ArrowHolder>> arrowHolders;
  std::vector<std::unique_ptr<ObjectQueue<Arrow>>> arrowQueues;
  std::unique_ptr<Feedback> feedback;
  std::unique_ptr<Combo> combo;
  std::unique_ptr<ComboDigit> digit1;
  std::unique_ptr<ComboDigit> digit2;
  std::unique_ptr<ComboDigit> digit3;
  u32 msecs = 0;
  bool started = false;
  u32 lastBeat = 0;

  void setUpBackground();
  void setUpArrows();
  void updateArrowHolders();
  void updateArrows(u32 millis);
  void processKeys(u16 keys);
};

#endif  // SONG_SCENE_H
