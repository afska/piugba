#include "SongScene.h"
#include <libgba-sprite-engine/gba/tonc_core.h>  // TODO: REMOVE tonc_core (qran_range)
#include <libgba-sprite-engine/palette/palette_manager.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/BeethovenVirus.h"
#include "data/content/compiled/shared_palette.h"
#include "utils/SpriteUtils.h"

const u32 BPM = 156;
const u32 INITIAL_OFFSET = 150;
const u32 ARROW_POOL_SIZE = 5;
const std::vector<ArrowType> ARROW_QUEUE_ORDER{
    ArrowType::DOWNLEFT, ArrowType::DOWNRIGHT, ArrowType::UPLEFT,
    ArrowType::UPRIGHT, ArrowType::CENTER};

std::vector<Background*> SongScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> SongScene::sprites() {
  std::vector<Sprite*> sprites;

  score->render(&sprites);

  u32 i = 0;
  for (auto& arrowQueue : arrowQueues) {
    arrowQueue->forEach(
        [&arrowQueue, &sprites](Arrow* it) { sprites.push_back(it->get()); });
    sprites.push_back(arrowHolders[i]->get());

    if (i == ARROW_QUEUE_ORDER.size() - 1)
      sprites.push_back(animation->get());

    i++;
  }

  return sprites;
}

void SongScene::load() {
  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>(new ForegroundPaletteManager(
          shared_palettePal, sizeof(shared_palettePal)));
  backgroundPalette =
      std::unique_ptr<BackgroundPaletteManager>(new BackgroundPaletteManager(
          BeethovenVirusPal, sizeof(BeethovenVirusPal)));
  SpriteBuilder<Sprite> builder;

  setUpBackground();
  setUpArrows();

  animation = std::unique_ptr<DanceAnimation>{
      new DanceAnimation(GBA_SCREEN_WIDTH * 1.75 / 3, ARROW_CORNER_MARGIN)};

  score = std::unique_ptr<Score>{new Score()};
}

void SongScene::setMsecs(u32 _msecs) {
  msecs = _msecs;
}

void SongScene::tick(u16 keys) {
  if (!started && msecs > INITIAL_OFFSET)
    started = true;
  u32 millis = started ? msecs - INITIAL_OFFSET : 0;
  // 60000-----BPMbeats
  // millis-----x = millis*BPM/60000
  u32 beat = (millis * BPM) / 60000;  // BPM bpm
  if (beat != lastBeat) {
    // int arrowIndex = qran_range(0, 4);
    // arrowQueues[arrowIndex]->push([](Arrow* it) { it->initialize(); });

    animation->update(beat);
  }
  lastBeat = beat;

  score->tick();
  updateArrowHolders();
  updateArrows(millis);
  processKeys(keys);
}

void SongScene::setUpBackground() {
  engine.get()->disableText();

  bg = std::unique_ptr<Background>(
      new Background(0, BeethovenVirusTiles, sizeof(BeethovenVirusTiles),
                     BeethovenVirusMap, sizeof(BeethovenVirusMap)));
  bg.get()->useMapScreenBlock(24);
}

void SongScene::setUpArrows() {
  for (auto& arrowType : ARROW_QUEUE_ORDER) {
    arrowQueues.push_back(std::unique_ptr<ObjectQueue<Arrow>>{
        new ObjectQueue<Arrow>(ARROW_POOL_SIZE, [&arrowType](u32 id) -> Arrow* {
          return new Arrow(id, arrowType);
        })});
    arrowHolders.push_back(
        std::unique_ptr<ArrowHolder>{new ArrowHolder(arrowType)});
  }
}

void SongScene::updateArrowHolders() {
  for (auto& it : arrowHolders)
    it->tick();
}

void SongScene::updateArrows(u32 millis) {
  int i = 0;
  for (auto& arrowQueue : arrowQueues) {
    arrowQueue->forEachActive([this, &arrowQueue, &millis, &i](Arrow* it) {
      bool isPressed = arrowHolders[i]->get()->getCurrentFrame() !=
                       ARROW_HOLDER_IDLE;  // TODO: Extract logic
      FeedbackType feedbackType = it->tick(millis, isPressed);
      if (feedbackType != FeedbackType::ACTIVE) {
        score->update(feedbackType);
        arrowQueue->pop();
      }
    });

    i++;
  }
}

void SongScene::processKeys(u16 keys) {
  if ((keys & KEY_DOWN) &&
      arrowHolders[0]->get()->getCurrentFrame() == ARROW_HOLDER_IDLE) {
    arrowQueues[0]->push([](Arrow* it) { it->initialize(); });
  }

  if ((keys & KEY_L) &&
      arrowHolders[2]->get()->getCurrentFrame() == ARROW_HOLDER_IDLE) {
    arrowQueues[2]->push([](Arrow* it) { it->initialize(); });
  }

  if ((((keys & KEY_B) | (keys & KEY_RIGHT))) &&
      arrowHolders[4]->get()->getCurrentFrame() == ARROW_HOLDER_IDLE) {
    arrowQueues[4]->push([](Arrow* it) { it->initialize(); });
  }

  if ((keys & KEY_R) &&
      arrowHolders[3]->get()->getCurrentFrame() == ARROW_HOLDER_IDLE) {
    arrowQueues[3]->push([](Arrow* it) { it->initialize(); });
  }

  if ((keys & KEY_A) &&
      arrowHolders[1]->get()->getCurrentFrame() == ARROW_HOLDER_IDLE) {
    arrowQueues[1]->push([](Arrow* it) { it->initialize(); });
  }

  SPRITE_goToFrame(arrowHolders[0]->get(),
                   keys & KEY_DOWN ? ARROW_HOLDER_PRESSED : ARROW_HOLDER_IDLE);
  SPRITE_goToFrame(arrowHolders[2]->get(),
                   keys & KEY_L ? ARROW_HOLDER_PRESSED : ARROW_HOLDER_IDLE);
  SPRITE_goToFrame(arrowHolders[4]->get(), (keys & KEY_B) | (keys & KEY_RIGHT)
                                               ? ARROW_HOLDER_PRESSED
                                               : ARROW_HOLDER_IDLE);
  SPRITE_goToFrame(arrowHolders[3]->get(),
                   keys & KEY_R ? ARROW_HOLDER_PRESSED : ARROW_HOLDER_IDLE);
  SPRITE_goToFrame(arrowHolders[1]->get(),
                   keys & KEY_A ? ARROW_HOLDER_PRESSED : ARROW_HOLDER_IDLE);
}
