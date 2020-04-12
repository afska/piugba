#include "SongScene.h"

#include <libgba-sprite-engine/effects/fade_out_scene.h>
#include <libgba-sprite-engine/palette/palette_manager.h>

#include "StageBreakScene.h"
#include "StartScene.h"  // TODO: REMOVE
#include "data/content/compiled/shared_palette.h"
#include "gameplay/Key.h"
#include "player/PlaybackState.h"
#include "utils/SpriteUtils.h"

extern "C" {
#include "player/player.h"
}

const u32 ARROW_POOL_SIZE = 45;

SongScene::SongScene(std::shared_ptr<GBAEngine> engine,
                     const GBFS_FILE* fs,
                     Song* song,
                     Chart* chart)
    : Scene(engine) {
  this->fs = fs;
  this->song = song;
  this->chart = chart;
}

std::vector<Background*> SongScene::backgrounds() {
  IFTEST { return {}; }
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
  player_play(song->audioPath.c_str());
  BACKGROUND1_DISABLE();
  BACKGROUND2_DISABLE();
  BACKGROUND3_DISABLE();

  setUpPalettes();
  IFNOTTEST { setUpBackground(); }
  setUpArrows();

  lifeBar = std::unique_ptr<LifeBar>(new LifeBar());
  score = std::unique_ptr<Score>{new Score(lifeBar.get())};

  judge =
      std::unique_ptr<Judge>(new Judge(arrowPool.get(), score.get(), [this]() {
        IFNOTTEST {
          unload();
          engine->transitionIntoScene(new StageBreakScene(engine),
                                      new FadeOutScene(2));
        }
      }));
  chartReader =
      std::unique_ptr<ChartReader>(new ChartReader(chart, judge.get()));
}

void SongScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (PlaybackState.hasFinished) {
    unload();
    engine->transitionIntoScene(new StartScene(engine), new FadeOutScene(2));
    return;
  }

  msecs = PlaybackState.msecs;
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

  u32 backgroundPaletteLength;
  auto backgroundPaletteData = (COLOR*)gbfs_get_obj(
      fs, song->backgroundPalettePath.c_str(), &backgroundPaletteLength);

  backgroundPalette =
      std::unique_ptr<BackgroundPaletteManager>(new BackgroundPaletteManager(
          backgroundPaletteData, backgroundPaletteLength));
}

void SongScene::setUpBackground() {
  u32 backgroundTilesLength, backgroundMapLength;
  auto backgroundTilesData = gbfs_get_obj(fs, song->backgroundTilesPath.c_str(),
                                          &backgroundTilesLength);
  auto backgroundMapData =
      gbfs_get_obj(fs, song->backgroundMapPath.c_str(), &backgroundMapLength);

  bg = std::unique_ptr<Background>(
      new Background(0, backgroundTilesData, backgroundTilesLength,
                     backgroundMapData, backgroundMapLength));
  bg.get()->useMapScreenBlock(24);
}

void SongScene::setUpArrows() {
  arrowPool = std::unique_ptr<ObjectPool<Arrow>>{new ObjectPool<Arrow>(
      ARROW_POOL_SIZE, [](u32 id) -> Arrow* { return new Arrow(id); })};

  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    arrowHolders.push_back(std::unique_ptr<ArrowHolder>{
        new ArrowHolder(static_cast<ArrowDirection>(i))});
}

void SongScene::updateArrowHolders() {
  for (auto& it : arrowHolders)
    it->tick();
}

void SongScene::updateArrows() {
  arrowPool->forEachActive([this](Arrow* it) {
    auto arrowState = it->tick(msecs, chartReader->hasStopped,
                               arrowHolders[it->direction]->getIsPressed());

    if (arrowState == ArrowState::OUT)
      judge->onOut(it);
    else if (arrowHolders[it->direction]->hasBeenPressedNow())
      judge->onPress(it);
  });
}

void SongScene::processKeys(u16 keys) {
  arrowHolders[0]->setIsPressed(KEY_DOWNLEFT(keys));
  arrowHolders[1]->setIsPressed(KEY_UPLEFT(keys));
  arrowHolders[2]->setIsPressed(KEY_CENTER(keys));
  arrowHolders[3]->setIsPressed(KEY_UPRIGHT(keys));
  arrowHolders[4]->setIsPressed(KEY_DOWNRIGHT(keys));

  IFKEYTEST {
    for (auto& arrowHolder : arrowHolders)
      if (arrowHolder->hasBeenPressedNow())
        arrowPool->create([&arrowHolder](Arrow* it) {
          it->initialize(ArrowType::UNIQUE, arrowHolder->direction);
        });
  }
}

void SongScene::unload() {
  player_stop();
  BACKGROUND1_ENABLE();
  BACKGROUND2_ENABLE();
  BACKGROUND3_ENABLE();
}

SongScene::~SongScene() {
  arrowHolders.clear();
  Song_free(song);
}
