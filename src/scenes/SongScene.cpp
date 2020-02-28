#include "SongScene.h"
#include <libgba-sprite-engine/palette/palette_manager.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/background.h"
#include "data/shared.h"

const u32 POOL_SIZE = 15;
const u32 BPM = 156;
const u32 INITIAL_OFFSET = 150;

std::vector<Background*> SongScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> SongScene::sprites() {
  std::vector<Sprite*> sprites;

  // sprites.push_back(animation->get());
  arrowPool->forEach([&sprites](Arrow* it) { sprites.push_back(it->get()); });
  for (auto& it : arrowHolders)
    sprites.push_back(it->get());

  return sprites;
}

void SongScene::load() {
  foregroundPalette = std::unique_ptr<ForegroundPaletteManager>(
      new ForegroundPaletteManager(sharedPal, sizeof(sharedPal)));
  backgroundPalette = std::unique_ptr<BackgroundPaletteManager>(
      new BackgroundPaletteManager(bg_palette, sizeof(bg_palette)));
  SpriteBuilder<Sprite> builder;

  setUpBackground();
  setUpArrowHolders();
  // animation = std::unique_ptr<DanceAnimation>{new DanceAnimation(95, 55)};
  arrowPool = std::unique_ptr<ObjectPool<Arrow>>{new ObjectPool<Arrow>(
      POOL_SIZE,
      [](u32 id) -> Arrow* { return new Arrow(id, ArrowType::UPRIGHT); })};
  arrowPool->create([](Arrow* it) { it->initialize(); });
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
    // animation->update(beat);
  }
  lastBeat = beat;

  processKeys(keys);
}

void SongScene::setUpBackground() {
  bg = std::unique_ptr<Background>(new Background(
      1, background_data, sizeof(background_data), map, sizeof(map)));
  bg.get()->useMapScreenBlock(16);
}

void SongScene::setUpArrowHolders() {
  arrowHolders.push_back(
      std::unique_ptr<ArrowHolder>{new ArrowHolder(ArrowType::DOWNLEFT)});
  arrowHolders.push_back(
      std::unique_ptr<ArrowHolder>{new ArrowHolder(ArrowType::UPLEFT)});
  arrowHolders.push_back(
      std::unique_ptr<ArrowHolder>{new ArrowHolder(ArrowType::CENTER)});
  arrowHolders.push_back(
      std::unique_ptr<ArrowHolder>{new ArrowHolder(ArrowType::UPRIGHT)});
  arrowHolders.push_back(
      std::unique_ptr<ArrowHolder>{new ArrowHolder(ArrowType::DOWNRIGHT)});
}

void SongScene::updateArrowHolders() {
  for (auto& it : arrowHolders)
    it->update();
}

void SongScene::updateArrows() {
  arrowPool->forEachActive([this](Arrow* it) {
    ArrowState arrowState = it->update();
    if (arrowState == ArrowState::OUT)
      arrowPool->discard(it->getId());
  });

  // auto it = arrows.begin();
  // while (it != arrows.end()) {
  //   ArrowState arrowState = (*it)->update();
  //   if (arrowState == ArrowState::OUT) {
  //     it = arrows.erase(it);
  //     engine->updateSpritesInScene();
  //   } else
  //     it++;
  // }
}

void SongScene::processKeys(u16 keys) {
  if (keys & KEY_B && arrowHolders[2]->get()->getCurrentFrame() == 0) {
    arrowPool->create([](Arrow* it) { it->initialize(); });
  }

  SpriteUtils::goToFrame(arrowHolders[0]->get(), keys & KEY_DOWN ? 1 : 0);
  SpriteUtils::goToFrame(arrowHolders[1]->get(), keys & KEY_L ? 1 : 0);
  SpriteUtils::goToFrame(arrowHolders[2]->get(),
                         (keys & KEY_B) | (keys & KEY_RIGHT) ? 1 : 0);
  SpriteUtils::goToFrame(arrowHolders[3]->get(), keys & KEY_R ? 1 : 0);
  SpriteUtils::goToFrame(arrowHolders[4]->get(), keys & KEY_A ? 1 : 0);
}
