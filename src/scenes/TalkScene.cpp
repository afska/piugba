#include "TalkScene.h"

#include "gameplay/Key.h"
#include "utils/SceneUtils.h"

const u32 BUTTON_MARGIN = 3;

std::vector<Sprite*> TalkScene::sprites() {
  auto sprites = TextScene::sprites();

  sprites.push_back(nextButton->get());

  return sprites;
}

void TalkScene::load() {
  TextScene::load();
  write(message);

  nextButton = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::CENTER, false, true)};
  nextButton->get()->moveTo(GBA_SCREEN_WIDTH - ARROW_SIZE - BUTTON_MARGIN,
                            GBA_SCREEN_HEIGHT - ARROW_SIZE - BUTTON_MARGIN);

  if (!withButton)
    SPRITE_hide(nextButton->get());
}

void TalkScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  TextScene::tick(keys);

  if (!(keys & KEY_ANY))
    canTriggerInput = true;

  if (SAVEFILE_isUsingGBAStyle())
    nextButton->setIsPressed(keys & KEY_A);
  else
    nextButton->setIsPressed(KEY_CENTER(keys));
  if (canTriggerInput && (keys & KEY_ANY) && (hasFinished() || skippable))
    onKeyPress(keys);

  nextButton->tick();
}
