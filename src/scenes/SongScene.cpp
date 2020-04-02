#include "SongScene.h"
#include <libgba-sprite-engine/palette/palette_manager.h>
#include "data/content/TestSong.h"
#include "data/content/compiled/shared_palette.h"
#include "gameplay/Key.h"

const u32 ARROW_POOL_SIZE = 45;

std::vector<Background*> SongScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> SongScene::sprites() {
  std::vector<Sprite*> sprites;

  sprites.push_back(lifeBar->get());
  score->render(&sprites);

  arrowPool->forEach([&sprites](Arrow* it) { sprites.push_back(it->get()); });

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

  lifeBar = std::unique_ptr<LifeBar>(new LifeBar());
  score = std::unique_ptr<Score>{new Score()};
}

void SongScene::tick(u16 keys) {
  bool isNewBeat = chartReader->update(msecs, arrowPool.get());

  if (isNewBeat)
    for (auto& arrowHolder : arrowHolders) {
      lifeBar->blink(foregroundPalette.get());
      if (!KEY_ANY_PRESSED(keys))
        arrowHolder->blink();
    }

  score->tick();
  lifeBar->tick(foregroundPalette.get());
  updateArrowHolders();
  updateArrows();
  processKeys(keys);
}

void SongScene::setUpPalettes() {
  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>(new ForegroundPaletteManager(
          shared_palettePal, sizeof(shared_palettePal)));
  backgroundPalette = std::unique_ptr<BackgroundPaletteManager>(
      new BackgroundPaletteManager(TestSongPal, sizeof(TestSongPal)));
}

void SongScene::setUpBackground() {
  engine.get()->disableText();

  bg = std::unique_ptr<Background>(
      new Background(0, TestSongTiles, sizeof(TestSongTiles), TestSongMap,
                     sizeof(TestSongMap)));
  bg.get()->useMapScreenBlock(24);
}

void SongScene::setUpArrows() {
  arrowPool = std::unique_ptr<ObjectPool<Arrow>>{new ObjectPool<Arrow>(
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
  arrowPool->forEachActive([this](Arrow* it) {
    bool isPressed = arrowHolders[it->type]->getIsPressed();

    FeedbackType feedbackType = it->tick(msecs, isPressed);
    if (feedbackType < FEEDBACK_TOTAL_SCORES)
      score->update(feedbackType);
    if (feedbackType == FeedbackType::ENDED)
      arrowPool->discard(it->id);
  });
}

void SongScene::processKeys(u16 keys) {
  if (arrowHolders[0]->setIsPressed(KEY_DOWNLEFT(keys))) {
    judge->onPress(ArrowType::DOWNLEFT, arrowPool.get(), score.get());
    IFTEST arrowPool->create(
        [](Arrow* it) { it->initialize(ArrowType::DOWNLEFT); });
  }

  if (arrowHolders[1]->setIsPressed(KEY_UPLEFT(keys))) {
    judge->onPress(ArrowType::UPLEFT, arrowPool.get(), score.get());
    IFTEST arrowPool->create(
        [](Arrow* it) { it->initialize(ArrowType::UPLEFT); });
  }

  if (arrowHolders[2]->setIsPressed(KEY_CENTER(keys))) {
    judge->onPress(ArrowType::CENTER, arrowPool.get(), score.get());
    IFTEST arrowPool->create(
        [](Arrow* it) { it->initialize(ArrowType::CENTER); });
  }

  if (arrowHolders[3]->setIsPressed(KEY_UPRIGHT(keys))) {
    judge->onPress(ArrowType::UPRIGHT, arrowPool.get(), score.get());
    IFTEST arrowPool->create(
        [](Arrow* it) { it->initialize(ArrowType::UPRIGHT); });
  }

  if (arrowHolders[4]->setIsPressed(KEY_DOWNRIGHT(keys))) {
    judge->onPress(ArrowType::DOWNRIGHT, arrowPool.get(), score.get());
    IFTEST arrowPool->create(
        [](Arrow* it) { it->initialize(ArrowType::DOWNRIGHT); });
  }
}

SongScene::~SongScene() {
  arrowHolders.clear();
}
