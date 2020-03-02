#include "SongScene.h"
#include <libgba-sprite-engine/palette/palette_manager.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/BeethovenVirus.h"
#include "data/shared.h"

const u32 POOL_SIZE = 5;
const u32 BPM = 156;
const u32 INITIAL_OFFSET = 150;

std::vector<Background*> SongScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> SongScene::sprites() {
  std::vector<Sprite*> sprites;

  sprites.push_back(animation->get());
  for (auto& arrowPool : arrowPools) {
    arrowPool->forEach([&arrowPool, &sprites](Arrow* it) {
      sprites.push_back(it->get());
    });
  }

  for (auto& it : arrowHolders)
    sprites.push_back(it->get());

  return sprites;
}

void SongScene::load() {
  foregroundPalette = std::unique_ptr<ForegroundPaletteManager>(
      new ForegroundPaletteManager(sharedPal, sizeof(sharedPal)));
  backgroundPalette =
      std::unique_ptr<BackgroundPaletteManager>(new BackgroundPaletteManager(
          BeethovenVirusPal, sizeof(BeethovenVirusPal)));
  SpriteBuilder<Sprite> builder;

  setUpBackground();
  setUpArrowHolders();

  animation = std::unique_ptr<DanceAnimation>{
      new DanceAnimation(GBA_SCREEN_WIDTH * 1.75 / 3, ARROW_CORNER_MARGIN)};

  for (int i = 0; i < ARROWS_TOTAL; i++)
    arrowPools.push_back(std::unique_ptr<ObjectPool<Arrow>>{
        new ObjectPool<Arrow>(POOL_SIZE, [i](u32 id) -> Arrow* {
          return new Arrow(id, static_cast<ArrowType>(i));
        })});
}

void SongScene::setMsecs(u32 _msecs) {
  msecs = _msecs;
}

void SongScene::tick(u16 keys) {
  updateArrowHolders();
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

  processKeys(keys);
}

void SongScene::setUpBackground() {
  engine.get()->disableText();

  bg = std::unique_ptr<Background>(
      new Background(0, BeethovenVirusTiles, sizeof(BeethovenVirusTiles),
                     BeethovenVirusMap, sizeof(BeethovenVirusMap)));
  bg.get()->useMapScreenBlock(24);
}

void SongScene::setUpArrowHolders() {
  for (int i = 0; i < ARROWS_TOTAL; i++)
    arrowHolders.push_back(std::unique_ptr<ArrowHolder>{
        new ArrowHolder(static_cast<ArrowType>(i))});
}

void SongScene::updateArrowHolders() {
  for (auto& it : arrowHolders)
    it->update();
}

void SongScene::updateArrows() {
  for (auto& arrowPool : arrowPools) {
    arrowPool->forEachActive([&arrowPool](Arrow* it) {
      ArrowState arrowState = it->update();
      if (arrowState == ArrowState::OUT)
        arrowPool->discard(it->getId());
    });
  }
}

void SongScene::processKeys(u16 keys) {
  if (keys & KEY_DOWN && arrowHolders[0]->get()->getCurrentFrame() == 0) {
    arrowPools[0]->create([](Arrow* it) { it->initialize(); });
  }

  if (keys & KEY_L && arrowHolders[1]->get()->getCurrentFrame() == 0) {
    arrowPools[1]->create([](Arrow* it) { it->initialize(); });
  }

  if (((keys & KEY_B) | (keys & KEY_RIGHT)) && arrowHolders[2]->get()->getCurrentFrame() == 0) {
    arrowPools[2]->create([](Arrow* it) { it->initialize(); });
  }

  if (keys & KEY_R && arrowHolders[3]->get()->getCurrentFrame() == 0) {
    arrowPools[3]->create([](Arrow* it) { it->initialize(); });
  }

  if (keys & KEY_A && arrowHolders[4]->get()->getCurrentFrame() == 0) {
    arrowPools[4]->create([](Arrow* it) { it->initialize(); });
  }

  SpriteUtils::goToFrame(arrowHolders[0]->get(), keys & KEY_DOWN ? 1 : 0);
  SpriteUtils::goToFrame(arrowHolders[1]->get(), keys & KEY_L ? 1 : 0);
  SpriteUtils::goToFrame(arrowHolders[2]->get(),
                         (keys & KEY_B) | (keys & KEY_RIGHT) ? 1 : 0);
  SpriteUtils::goToFrame(arrowHolders[3]->get(), keys & KEY_R ? 1 : 0);
  SpriteUtils::goToFrame(arrowHolders[4]->get(), keys & KEY_A ? 1 : 0);
}
