#ifndef DEATH_MIX_SCENE_H
#define DEATH_MIX_SCENE_H

#include "TalkScene.h"
#include "gameplay/SequenceMessages.h"

class DeathMixScene : public TalkScene {
 public:
  DeathMixScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs)
      : TalkScene(
            engine,
            fs,
            MODE_DEATH_MIX,
            [this](u16 keys) { confirm(keys); },
            true) {}

  virtual std::vector<Sprite*> sprites() override;

 protected:
  void load() override;
  void tick(u16 keys) override;

 private:
  void confirm(u16 keys);
};

#endif  // DEATH_MIX_SCENE_H
