#include "SongScene.h"
#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/palette/palette_manager.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/arrow_center.h"
#include "data/arrow_center_placeholder.h"
#include "data/arrow_downleft.h"
#include "data/arrow_downleft_placeholder.h"
#include "data/arrow_upleft.h"
#include "data/arrow_upleft_placeholder.h"
#include "data/background.h"
#include "data/shared.h"

const u32 BPM = 156;
const u32 INITIAL_OFFSET = 150;

std::vector<Background*> SongScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> SongScene::sprites() {
  std::vector<Sprite*> sprites;

  sprites.push_back(animation->get());
  for (auto it = arrows.begin(); it != arrows.end(); it++) {
    sprites.push_back((*it)->get());
  }
  sprites.push_back(a1.get());
  sprites.push_back(a2.get());
  sprites.push_back(a3.get());
  sprites.push_back(a4.get());
  sprites.push_back(a5.get());

  return sprites;
}

void SongScene::load() {
  foregroundPalette = std::unique_ptr<ForegroundPaletteManager>(
      new ForegroundPaletteManager(sharedPal, sizeof(sharedPal)));
  backgroundPalette = std::unique_ptr<BackgroundPaletteManager>(
      new BackgroundPaletteManager(bg_palette, sizeof(bg_palette)));
  SpriteBuilder<Sprite> builder;

  TextStream::instance().setText("AHI TA VITEH!", 5, 10);

  bg = std::unique_ptr<Background>(new Background(
      1, background_data, sizeof(background_data), map, sizeof(map)));
  bg.get()->useMapScreenBlock(16);

  a1 = builder
           .withData(arrow_downleft_placeholderTiles,
                     sizeof(arrow_downleft_placeholderTiles))
           .withSize(SIZE_16_16)
           .withLocation(ARROW_CORNER_MARGIN + ARROW_MARGIN * 0,
                         ARROW_CORNER_MARGIN)
           .buildPtr();
  a2 = builder
           .withData(arrow_upleft_placeholderTiles,
                     sizeof(arrow_upleft_placeholderTiles))
           .withSize(SIZE_16_16)
           .withLocation(ARROW_CORNER_MARGIN + ARROW_MARGIN * 1,
                         ARROW_CORNER_MARGIN)
           .buildPtr();
  a3 = builder
           .withData(arrow_center_placeholderTiles,
                     sizeof(arrow_center_placeholderTiles))
           .withSize(SIZE_16_16)
           .withLocation(ARROW_CORNER_MARGIN + ARROW_MARGIN * 2,
                         ARROW_CORNER_MARGIN)
           .buildPtr();
  a4 = builder
           .withData(arrow_upleft_placeholderTiles,
                     sizeof(arrow_upleft_placeholderTiles))
           .withSize(SIZE_16_16)
           .withLocation(ARROW_CORNER_MARGIN + ARROW_MARGIN * 3,
                         ARROW_CORNER_MARGIN)
           .buildPtr();
  a5 = builder
           .withData(arrow_downleft_placeholderTiles,
                     sizeof(arrow_downleft_placeholderTiles))
           .withSize(SIZE_16_16)
           .withLocation(ARROW_CORNER_MARGIN + ARROW_MARGIN * 4,
                         ARROW_CORNER_MARGIN)
           .buildPtr();

  animation = std::unique_ptr<DanceAnimation>{new DanceAnimation(95, 55)};

  arrows.push_back(std::unique_ptr<Arrow>{new Arrow(ArrowType::UPRIGHT)});
}

void SongScene::setMsecs(u32 _msecs) {
  msecs = _msecs;
}

void SongScene::tick(u16 keys) {
  a4->flipHorizontally(true);
  a5->flipHorizontally(true);

  updateArrows();

  if (!started && msecs > INITIAL_OFFSET)
    started = true;

  u32 millis = started ? msecs - INITIAL_OFFSET : 0;

  // 60000-----BPMbeats
  // millis-----x = millis*BPM/60000
  u32 beat = (millis * BPM) / 60000;  // BPM bpm
  if (beat != lastBeat) {
    animation->update(beat);
  }
  lastBeat = beat;

  TextStream::instance().setText(std::to_string(beat), 9, 15);

  int is_odd = beat & 1;
  TextStream::instance().setText("----------", !is_odd ? 19 : 18, 1);
  TextStream::instance().setText("oooooooooo", is_odd ? 19 : 18, 1);

  a1->makeAnimated(0, 0, 0);
  a1->stopAnimating();
  a1->animateToFrame(keys & KEY_DOWN ? 1 : 0);

  a2->makeAnimated(0, 0, 0);
  a2->stopAnimating();
  a2->animateToFrame(keys & KEY_L ? 1 : 0);

  a3->makeAnimated(0, 0, 0);
  a3->stopAnimating();
  a3->animateToFrame((keys & KEY_B) | (keys & KEY_RIGHT) ? 1 : 0);

  a4->makeAnimated(0, 0, 0);
  a4->stopAnimating();
  a4->animateToFrame(keys & KEY_R ? 1 : 0);

  a5->makeAnimated(0, 0, 0);
  a5->stopAnimating();
  a5->animateToFrame(keys & KEY_A ? 1 : 0);
}

void SongScene::updateArrows() {
  auto it = arrows.begin();
  while (it != arrows.end()) {
    ArrowState arrowState = (*it)->update();
    if (arrowState == ArrowState::OUT) {
      it = arrows.erase(it);
      this->engine->updateSpritesInScene();
    } else
      it++;
  }
}
