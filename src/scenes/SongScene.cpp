#include "SongScene.h"
#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/palette/palette_manager.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/arrow_downleft_placeholder.h"
#include "data/arrow_upleft_placeholder.h"
#include "data/arrow_center_placeholder.h"
#include "data/arrow_center.h"
#include "data/arrow_downleft.h"
#include "data/arrow_upleft.h"
#include "data/shared.h"

std::vector<Background*> SongScene::backgrounds() {
  return {};
}

std::vector<Sprite*> SongScene::sprites() {
  return {animation.get(), a1.get(), a2.get(), a3.get(), a4.get(), a5.get()};
}

unsigned int msecs;
bool started = false;
unsigned int initial_offset = 150;
unsigned int last_beat = 0;
int count = -3;
int velocity = 10;
bool to_left = false;

void SongScene::load() {
  foregroundPalette = std::unique_ptr<ForegroundPaletteManager>(
      new ForegroundPaletteManager(sharedPal, sizeof(sharedPal)));
  backgroundPalette = std::unique_ptr<BackgroundPaletteManager>(
      new BackgroundPaletteManager(sharedPal, sizeof(sharedPal)));
  SpriteBuilder<Sprite> builder;

  TextStream::instance().setText("AHI TA VITEH!", 5, 8);

  int marginBorder = 4;
  int margin = 16 + 2;
  a1 = builder.withData(arrow_downleft_placeholderTiles, sizeof(arrow_downleft_placeholderTiles))
           .withSize(SIZE_16_16)
           .withLocation(marginBorder + margin * 0, marginBorder)
           .buildPtr();
  a2 = builder.withData(arrow_upleft_placeholderTiles, sizeof(arrow_upleft_placeholderTiles))
           .withSize(SIZE_16_16)
           .withLocation(marginBorder + margin * 1, marginBorder)
           .buildPtr();
  a3 = builder.withData(arrow_center_placeholderTiles, sizeof(arrow_center_placeholderTiles))
           .withSize(SIZE_16_16)
           .withLocation(marginBorder + margin * 2, marginBorder)
           .buildPtr();
  a4 = builder.withData(arrow_upleft_placeholderTiles, sizeof(arrow_upleft_placeholderTiles))
           .withSize(SIZE_16_16)
           .withLocation(marginBorder + margin * 3, marginBorder)
           .buildPtr();
  a5 = builder.withData(arrow_downleft_placeholderTiles, sizeof(arrow_downleft_placeholderTiles))
           .withSize(SIZE_16_16)
           .withLocation(marginBorder + margin * 4, marginBorder)
           .buildPtr();

  animation = builder.withData(arrow_centerTiles, sizeof(arrow_centerTiles))
                  .withSize(SIZE_16_16)
                  .withAnimated(5, 2)
                  .withLocation(78, 55)
                  .buildPtr();
}

void SongScene::setMsecs(unsigned int _msecs) {
  msecs = _msecs;
}

void SongScene::tick(u16 keys) {
  a4->flipHorizontally(true);
  a5->flipHorizontally(true);

  if (!started && msecs > initial_offset)
    started = true;

  unsigned int millis = started ? msecs - initial_offset : 0;

  // 60000-----BPMbeats
  // millis-----x = millis*BPM/60000
  unsigned int beat = (millis * 156) / 60000;  // BPM bpm
  if (beat != last_beat) {
    int delta = sgn(velocity);
    count += delta;
    animation->moveTo(animation->getX() + velocity, animation->getY());
    if (abs(count) >= 4) {
      velocity = -velocity;
      to_left = !to_left;
      animation->flipHorizontally(to_left);
    }
  }

  TextStream::instance().setText(std::to_string(count), 10, 1);
  last_beat = beat;

  int is_odd = beat & 1;
  TextStream::instance().setText("----------", !is_odd ? 18 : 17, 1);
  TextStream::instance().setText("oooooooooo", is_odd ? 18 : 17, 1);

  // if (keys & KEY_LEFT) {
  //   animation->flipHorizontally(true);
  // } else if (keys & KEY_RIGHT) {
  //   animation->flipHorizontally(false);
  // } else if (keys & KEY_UP) {
  //   animation->flipVertically(true);
  // } else if (keys & KEY_DOWN) {
  //   animation->flipVertically(false);
  // }
}
