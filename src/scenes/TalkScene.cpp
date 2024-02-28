#include "TalkScene.h"

#include "gameplay/Key.h"
#include "utils/SceneUtils.h"

const u32 BUTTON_MARGIN = 3;

std::vector<Sprite*> TalkScene::sprites() {
  auto sprites = TextScene::sprites();

  sprites.push_back(confirmButton->get());

  return sprites;
}

void TalkScene::load() {
  TextScene::load();
  write(message);

  confirmButton = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::CENTER, false, true)};
  confirmButton->get()->moveTo(GBA_SCREEN_WIDTH - ARROW_SIZE - BUTTON_MARGIN,
                               GBA_SCREEN_HEIGHT - ARROW_SIZE - BUTTON_MARGIN);

  if (!withButton)
    SPRITE_hide(confirmButton->get());
}

void TalkScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  TextScene::tick(keys);

  if (!KEY_ANYKEY(keys))
    canTriggerInput = true;

  if (SAVEFILE_isUsingGBAStyle())
    confirmButton->setIsPressed(keys & KEY_A);
  else
    confirmButton->setIsPressed(KEY_CENTER(keys));
  if (canTriggerInput && KEY_ANYKEY(keys) && (hasFinished() || skippable))
    onKeyPress(keys);

  confirmButton->tick();
}
