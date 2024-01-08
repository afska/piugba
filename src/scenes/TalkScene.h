#ifndef TALK_SCENE_H
#define TALK_SCENE_H

#include <functional>
#include <string>

#include "base/TextScene.h"
#include "objects/ui/ArrowSelector.h"

class TalkScene : public TextScene {
 public:
  bool withButton = true;

  explicit TalkScene(std::shared_ptr<GBAEngine> engine,
                     const GBFS_FILE* fs,
                     std::string message,
                     std::function<void(u16 keys)> onKeyPress,
                     bool skippable = false)
      : TextScene(engine, fs) {
    this->message = message;
    this->onKeyPress = onKeyPress;
    this->skippable = skippable;
  }

 protected:
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

 private:
  std::string message;
  std::function<void(u16 keys)> onKeyPress;
  std::unique_ptr<ArrowSelector> confirmButton;
  bool skippable;
  bool canTriggerInput = false;
};

#endif  // TALK_SCENE_H
