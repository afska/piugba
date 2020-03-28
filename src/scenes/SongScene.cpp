#include "SongScene.h"
#include <libgba-sprite-engine/palette/palette_manager.h>
#include "data/content/BeethovenVirus.h"
#include "data/content/compiled/shared_palette.h"
#include "utils/SpriteUtils.h"

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
  chartReader = std::unique_ptr<ChartReader>(new ChartReader(chart));
  judge = std::unique_ptr<Judge>(new Judge());

  setUpPalettes();
  setUpBackground();
  setUpArrows();

  score = std::unique_ptr<Score>{new Score()};
}

void SongScene::tick(u16 keys) {
  bool isNewBeat = chartReader->update(msecs, arrowQueue.get());

  if (isNewBeat)
    for (auto& arrowHolder : arrowHolders)
      arrowHolder->blink();

  score->tick();
  updateArrowHolders();
  updateArrows();
  processKeys(keys);
}

void SongScene::setUpPalettes() {
  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>(new ForegroundPaletteManager(
          shared_palettePal, sizeof(shared_palettePal)));
  backgroundPalette =
      std::unique_ptr<BackgroundPaletteManager>(new BackgroundPaletteManager(
          BeethovenVirusPal, sizeof(BeethovenVirusPal)));
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

void SongScene::updateArrows() {
  arrowQueue->forEachActive([this](Arrow* it) {
    bool isPressed = arrowHolders[it->type]->getIsPressed();

    FeedbackType feedbackType = it->tick(msecs, isPressed);
    if (feedbackType < FEEDBACK_TOTAL_SCORES)
      score->update(feedbackType);
    if (feedbackType == FeedbackType::INACTIVE)
      arrowQueue->pop();
  });
}

void SongScene::processKeys(u16 keys) {
  // if (((keys & KEY_DOWN) | (keys & KEY_LEFT)) &&
  //     !arrowHolders[0]->getIsPressed()) {
  //   arrowQueue->push([](Arrow* it) { it->initialize(ArrowType::DOWNLEFT); });
  // }
  // if (((keys & KEY_L) | (keys & KEY_UP)) && !arrowHolders[1]->getIsPressed())
  // {
  //   arrowQueue->push([](Arrow* it) { it->initialize(ArrowType::UPLEFT); });
  // }
  // if (((keys & KEY_B) | (keys & KEY_RIGHT)) &&
  //     !arrowHolders[2]->getIsPressed()) {
  //   arrowQueue->push([](Arrow* it) { it->initialize(ArrowType::CENTER); });
  // }
  // if (keys & KEY_R && !arrowHolders[3]->getIsPressed()) {
  //   arrowQueue->push([](Arrow* it) { it->initialize(ArrowType::UPRIGHT); });
  // }
  // if (keys & KEY_A && !arrowHolders[4]->getIsPressed()) {
  //   arrowQueue->push([](Arrow* it) { it->initialize(ArrowType::DOWNRIGHT);
  //   });
  // }

  if (arrowHolders[0]->setIsPressed((keys & KEY_DOWN) | (keys & KEY_LEFT)))
    judge->onPress(ArrowType::DOWNLEFT);

  if (arrowHolders[1]->setIsPressed((keys & KEY_L) | (keys & KEY_UP)))
    judge->onPress(ArrowType::UPLEFT);

  if (arrowHolders[2]->setIsPressed((keys & KEY_B) | (keys & KEY_RIGHT)))
    judge->onPress(ArrowType::CENTER);

  if (arrowHolders[3]->setIsPressed(keys & KEY_R))
    judge->onPress(ArrowType::UPRIGHT);

  if (arrowHolders[4]->setIsPressed(keys & KEY_A))
    judge->onPress(ArrowType::DOWNRIGHT);
}

SongScene::~SongScene() {
  arrowHolders.clear();
}
