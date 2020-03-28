#include "SongScene.h"
#include <libgba-sprite-engine/palette/palette_manager.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/BeethovenVirus.h"
#include "data/content/compiled/shared_palette.h"
#include "utils/SpriteUtils.h"

const u32 BPM = 162;
const int INITIAL_OFFSET = 175;
const u32 ARROW_POOL_SIZE = 20;

std::vector<Background*> SongScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> SongScene::sprites() {
  std::vector<Sprite*> sprites;

  score->render(&sprites);

  arrowQueue->forEach([&sprites](Arrow* it) { sprites.push_back(it->get()); });

  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    sprites.push_back(arrowHolders[i]->get());

  return sprites;
}

void SongScene::load() {
  // TODO: REMOVE
  Song* song = Song_parse(fs, (char*)"beethoven-virus.pius");
  u32 index = 0;
  for (u32 i = 0; i < song->chartCount; i++) {
    if (song->charts[i].level == 7) {
      index = i;
      break;
    }
  }
  chartReader =
      std::unique_ptr<ChartReader>(new ChartReader(song->charts + index));
  // Song_free(song);

  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>(new ForegroundPaletteManager(
          shared_palettePal, sizeof(shared_palettePal)));
  backgroundPalette =
      std::unique_ptr<BackgroundPaletteManager>(new BackgroundPaletteManager(
          BeethovenVirusPal, sizeof(BeethovenVirusPal)));
  SpriteBuilder<Sprite> builder;

  setUpBackground();
  setUpArrows();

  score = std::unique_ptr<Score>{new Score()};
}

void SongScene::tick(u16 keys) {
  chartReader->update(msecs, arrowQueue.get());

  // 60000-----BPMbeats
  // millis-----x = millis*BPM/60000
  int millis = msecs - INITIAL_OFFSET;
  int beat = Div(millis * BPM, 60000);  // BPM bpm
  if (beat != lastBeat) {
    // TODO: ANIMATE
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
  arrowQueue = std::unique_ptr<ObjectQueue<Arrow>>{new ObjectQueue<Arrow>(
      ARROW_POOL_SIZE, [](u32 id) -> Arrow* { return new Arrow(id); })};

  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    arrowHolders.push_back(std::unique_ptr<ArrowHolder>{
        new ArrowHolder(static_cast<ArrowType>(i))});
}

void SongScene::updateArrowHolders() {
  for (auto& it : arrowHolders)
    it->tick();
}

void SongScene::updateArrows(u32 millis) {
  arrowQueue->forEachActive([this, &millis](Arrow* it) {
    bool isPressed = arrowHolders[it->type]->getIsPressed();

    FeedbackType feedbackType = it->tick(millis, isPressed);
    if (feedbackType < FEEDBACK_TOTAL_SCORES)
      score->update(feedbackType);
    if (feedbackType == FeedbackType::INACTIVE)
      arrowQueue->pop();
  });
}

void SongScene::processKeys(u16 keys) {
  arrowHolders[0]->setIsPressed((keys & KEY_DOWN) | (keys & KEY_LEFT));
  arrowHolders[1]->setIsPressed((keys & KEY_L) | (keys & KEY_UP));
  arrowHolders[2]->setIsPressed((keys & KEY_B) | (keys & KEY_RIGHT));
  arrowHolders[3]->setIsPressed(keys & KEY_R);
  arrowHolders[4]->setIsPressed(keys & KEY_A);
}

SongScene::~SongScene() {
  arrowHolders.clear();
}
