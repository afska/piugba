#include "SongScene.h"
#include <libgba-sprite-engine/palette/palette_manager.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/BeethovenVirus.h"
#include "data/content/compiled/shared_palette.h"

const u32 ARROW_QUEUE_SIZE = 5;
const u32 BPM = 156;
const u32 INITIAL_OFFSET = 150;

std::vector<Background*> SongScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> SongScene::sprites() {
  std::vector<Sprite*> sprites;

  sprites.push_back(feedback->get());
  sprites.push_back(combo->get());
  sprites.push_back(digit1->get());
  sprites.push_back(digit2->get());
  sprites.push_back(digit3->get());
  sprites.push_back(animation->get());

  for (auto& arrowQueue : arrowQueues) {
    arrowQueue->forEach(
        [&arrowQueue, &sprites](Arrow* it) { sprites.push_back(it->get()); });
  }

  for (auto& it : arrowHolders)
    sprites.push_back(it->get());

  return sprites;
}

void SongScene::load() {
  foregroundPalette = std::unique_ptr<ForegroundPaletteManager>(
      new ForegroundPaletteManager(shared_palettePal, sizeof(shared_palettePal)));
  backgroundPalette =
      std::unique_ptr<BackgroundPaletteManager>(new BackgroundPaletteManager(
          BeethovenVirusPal, sizeof(BeethovenVirusPal)));
  SpriteBuilder<Sprite> builder;

  setUpBackground();
  setUpArrowHolders();

  animation = std::unique_ptr<DanceAnimation>{
      new DanceAnimation(GBA_SCREEN_WIDTH * 1.75 / 3, ARROW_CORNER_MARGIN)};

  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    arrowQueues.push_back(std::unique_ptr<ObjectQueue<Arrow>>{
        new ObjectQueue<Arrow>(ARROW_QUEUE_SIZE, [i](u32 id) -> Arrow* {
          return new Arrow(id, static_cast<ArrowType>(i));
        })});

  feedback = std::unique_ptr<Feedback>{new Feedback(FeedbackType::PERFECT)};

  combo = std::unique_ptr<Combo>{new Combo()};

  digit1 = std::unique_ptr<ComboDigit>{new ComboDigit(0, 0)};
  digit2 = std::unique_ptr<ComboDigit>{new ComboDigit(0, 1)};
  digit3 = std::unique_ptr<ComboDigit>{new ComboDigit(1, 2)};
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
  if (beat != lastBeat)
    animation->update(beat);
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
  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    arrowHolders.push_back(std::unique_ptr<ArrowHolder>{
        new ArrowHolder(static_cast<ArrowType>(i))});
}

void SongScene::updateArrowHolders() {
  for (auto& it : arrowHolders)
    it->update();
}

void SongScene::updateArrows() {
  for (auto& arrowQueue : arrowQueues) {
    arrowQueue->forEachActive([&arrowQueue](Arrow* it) {
      ArrowState arrowState = it->update();
      if (arrowState == ArrowState::OUT)
        arrowQueue->pop();
    });
  }
}

void SongScene::processKeys(u16 keys) {
  if (keys & KEY_DOWN && arrowHolders[0]->get()->getCurrentFrame() == 0) {
    arrowQueues[0]->push([](Arrow* it) { it->initialize(); });
  }

  if (keys & KEY_L && arrowHolders[1]->get()->getCurrentFrame() == 0) {
    arrowQueues[1]->push([](Arrow* it) { it->initialize(); });
  }

  if (((keys & KEY_B) | (keys & KEY_RIGHT)) &&
      arrowHolders[2]->get()->getCurrentFrame() == 0) {
    arrowQueues[2]->push([](Arrow* it) { it->initialize(); });
  }

  if (keys & KEY_R && arrowHolders[3]->get()->getCurrentFrame() == 0) {
    arrowQueues[3]->push([](Arrow* it) { it->initialize(); });
  }

  if (keys & KEY_A && arrowHolders[4]->get()->getCurrentFrame() == 0) {
    arrowQueues[4]->push([](Arrow* it) { it->initialize(); });
  }

  SpriteUtils::goToFrame(arrowHolders[0]->get(), keys & KEY_DOWN ? 1 : 0);
  SpriteUtils::goToFrame(arrowHolders[1]->get(), keys & KEY_L ? 1 : 0);
  SpriteUtils::goToFrame(arrowHolders[2]->get(),
                         (keys & KEY_B) | (keys & KEY_RIGHT) ? 1 : 0);
  SpriteUtils::goToFrame(arrowHolders[3]->get(), keys & KEY_R ? 1 : 0);
  SpriteUtils::goToFrame(arrowHolders[4]->get(), keys & KEY_A ? 1 : 0);
}
