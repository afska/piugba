#include "SongScene.h"

#include <libgba-sprite-engine/palette/palette_manager.h>

#include "data/content/TestSong.h"
#include "data/content/compiled/shared_palette.h"
#include "gameplay/Key.h"

const u32 ARROW_POOL_SIZE = 45;

SongScene::SongScene(std::shared_ptr<GBAEngine> engine, Chart* chart)
    : Scene(engine) {
  this->chart = chart;
}

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
  setUpPalettes();
  setUpBackground();
  setUpArrows();

  lifeBar = std::unique_ptr<LifeBar>(new LifeBar());
  score = std::unique_ptr<Score>{new Score()};

  chartReader = std::unique_ptr<ChartReader>(new ChartReader(chart));
  judge = std::unique_ptr<Judge>(new Judge(arrowPool.get(), score.get()));
}

void SongScene::tick(u16 keys) {
  bool isNewBeat = chartReader->update(msecs, arrowPool.get());

  if (isNewBeat)
    for (auto& arrowHolder : arrowHolders) {
      lifeBar->blink(foregroundPalette.get());
      if (!KEY_ANY_PRESSED(keys))
        arrowHolder->blink();
    }

  updateArrowHolders();
  processKeys(keys);
  updateArrows();
  score->tick();
  lifeBar->tick(foregroundPalette.get());
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
    auto arrowState = it->tick(msecs);

    if (arrowState == ArrowState::OUT)
      judge->onOut(it);
    else if (arrowHolders[it->type]->hasBeenPressedNow())
      judge->onPress(it);
  });
}

void SongScene::processKeys(u16 keys) {
  arrowHolders[0]->setIsPressed(KEY_DOWNLEFT(keys));
  arrowHolders[1]->setIsPressed(KEY_UPLEFT(keys));
  arrowHolders[2]->setIsPressed(KEY_CENTER(keys));
  arrowHolders[3]->setIsPressed(KEY_UPRIGHT(keys));
  arrowHolders[4]->setIsPressed(KEY_DOWNRIGHT(keys));

  IFTEST {
    for (auto& arrowHolder : arrowHolders)
      if (arrowHolder->hasBeenPressedNow())
        arrowPool->create(
            [&arrowHolder](Arrow* it) { it->initialize(arrowHolder->type); });
  }
}

SongScene::~SongScene() {
  arrowHolders.clear();
}
