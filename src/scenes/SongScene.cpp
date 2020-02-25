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
  return {animation->get(), a1.get(), a2.get(), a3.get(), a4.get(), a5.get()};
}

const u32 BPM = 156;
const u32 INITIAL_OFFSET = 150;

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

  animation = std::unique_ptr<DanceAnimation> { new DanceAnimation(78, 55) };
}

void SongScene::setMsecs(u32 _msecs) {
  msecs = _msecs;
}

void SongScene::tick(u16 keys) {
  a4->flipHorizontally(true);
  a5->flipHorizontally(true);

  if (!started && msecs > INITIAL_OFFSET)
    started = true;

  u32 millis = started ? msecs - INITIAL_OFFSET : 0;

  // 60000-----BPMbeats
  // millis-----x = millis*BPM/60000
  u32 beat = (millis * BPM) / 60000;  // BPM bpm
  if (beat != lastBeat) animation->update(beat);
  lastBeat = beat;

  TextStream::instance().setText(std::to_string(beat), 10, 1);

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
