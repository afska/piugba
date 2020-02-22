#include <libgba-sprite-engine/palette/palette_manager.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include <libgba-sprite-engine/background/text_stream.h>
#include "SongScene.h"
#include "data/lama.h"

std::vector<Background*> SongScene::backgrounds() {
  return {};
}

std::vector<Sprite*> SongScene::sprites() {
  return {animation.get()};
}

void SongScene::load() {
  foregroundPalette = std::unique_ptr<ForegroundPaletteManager>(
      new ForegroundPaletteManager(sharedPal, sizeof(sharedPal)));
  backgroundPalette =
      std::unique_ptr<BackgroundPaletteManager>(new BackgroundPaletteManager());

  SpriteBuilder<Sprite> builder;

  animation = builder.withData(lamaTiles, sizeof(lamaTiles))
                  .withSize(SIZE_32_32)
                  .withAnimated(6, 3)
                  .withLocation(50, 50)
                  .buildPtr();

  TextStream::instance().setText("AHI TA VITEH!", 3, 8);

  engine->getTimer()->start();
}

void SongScene::tick(u16 keys) {
  TextStream::instance().setText(engine->getTimer()->to_string(), 18, 1);

  if (keys & KEY_LEFT) {
    animation->flipHorizontally(true);
  } else if (keys & KEY_RIGHT) {
    animation->flipHorizontally(false);
  } else if (keys & KEY_UP) {
    animation->flipVertically(true);
  } else if (keys & KEY_DOWN) {
    animation->flipVertically(false);
  }
}
