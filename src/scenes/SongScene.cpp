#include "SongScene.h"

#include <libgba-sprite-engine/effects/fade_out_scene.h>
#include <libgba-sprite-engine/palette/palette_manager.h>

#include "SelectionScene.h"
#include "StageBreakScene.h"
#include "data/content/_compiled_sprites/palette_song.h"
#include "gameplay/Key.h"
#include "player/PlaybackState.h"
#include "ui/Darkener.h"
#include "utils/BackgroundUtils.h"
#include "utils/EffectUtils.h"
#include "utils/SpriteUtils.h"

extern "C" {
#include "player/player.h"
}

const u32 ID_DARKENER = 0;
const u32 ID_MAIN_BACKGROUND = 1;
const u32 ARROW_POOL_SIZE = 50;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 24;

static std::unique_ptr<Darkener> darkener{new Darkener(ID_DARKENER)};

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

  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    sprites.push_back(fakeHeads[i]->get());

  arrowPool->forEach([&sprites](Arrow* it) { sprites.push_back(it->get()); });

  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    sprites.push_back(arrowHolders[i]->get());

  return sprites;
}

void SongScene::load() {
  player_play(song->audioPath.c_str());

  BACKGROUND_enable(false, false, false, false);
  IFTEST { BACKGROUND_enable(true, false, false, false); }

  setUpPalettes();
  IFNOTTEST { setUpBackground(); }
  setUpArrows();

  lifeBar = std::unique_ptr<LifeBar>(new LifeBar());
  score = std::unique_ptr<Score>{new Score(lifeBar.get())};

  judge = std::unique_ptr<Judge>(
      new Judge(arrowPool.get(), &arrowHolders, score.get(), [this]() {
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

  if (!hasStarted) {
    IFNOTTEST {
      BACKGROUND_enable(true, true, false, false);
      darkener->initialize();
    }
    hasStarted = true;
  }

  msecs = (int)PlaybackState.msecs;

  if (PlaybackState.hasFinished || msecs >= (int)song->lastMillisecond) {
    unload();
    engine->transitionIntoScene(new SelectionScene(engine),
                                new FadeOutScene(2));
    return;
  }

  bool isNewBeat = chartReader->update(&this->msecs, arrowPool.get());

  if (isNewBeat)
    for (auto& arrowHolder : arrowHolders) {
      lifeBar->blink(foregroundPalette.get());
      if (!KEY_ANY_PRESSED(keys))
        arrowHolder->blink();
    }

  updateArrowHolders();
  processKeys(keys);
  updateFakeHeads();
  updateArrows();
  score->tick();
  lifeBar->tick(foregroundPalette.get());
}

void SongScene::setUpPalettes() {
  foregroundPalette = std::unique_ptr<ForegroundPaletteManager>(
      new ForegroundPaletteManager(palette_songPal, sizeof(palette_songPal)));

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

  bg = std::unique_ptr<Background>(new Background(
      ID_MAIN_BACKGROUND, backgroundTilesData, backgroundTilesLength,
      backgroundMapData, backgroundMapLength));
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
  bg->setMosaic(true);
}

void SongScene::setUpArrows() {
  arrowPool = std::unique_ptr<ObjectPool<Arrow>>{new ObjectPool<Arrow>(
      ARROW_POOL_SIZE, [](u32 id) -> Arrow* { return new Arrow(id); })};

  for (u32 i = 0; i < ARROWS_TOTAL; i++) {
    arrowHolders.push_back(std::unique_ptr<ArrowHolder>{
        new ArrowHolder(static_cast<ArrowDirection>(i))});

    auto fakeHead =
        std::unique_ptr<Arrow>{new Arrow(ARROW_TILEMAP_LOADING_ID + i)};
    SPRITE_hide(fakeHead->get());
    fakeHeads.push_back(std::move(fakeHead));
  }
}

void SongScene::updateArrowHolders() {
  for (auto& it : arrowHolders)
    it->tick();
}

void SongScene::updateArrows() {
  arrowPool->forEachActive([this](Arrow* it) {
    ArrowDirection direction = it->direction;
    bool isPressing = arrowHolders[direction]->getIsPressed();

    ArrowState arrowState = it->tick(chartReader->hasStopped, isPressing);

    if (arrowState == ArrowState::OUT)
      judge->onOut(it);
    else if (arrowHolders[direction]->hasBeenPressedNow())
      judge->onPress(it);
  });
}

void SongScene::updateFakeHeads() {
  for (u32 i = 0; i < ARROWS_TOTAL; i++) {
    auto direction = static_cast<ArrowDirection>(i);

    bool isHoldMode = false;
    chartReader->withNextHoldArrow(
        direction, [&isHoldMode, this](HoldArrow* holdArrow) {
          isHoldMode = msecs >= holdArrow->startTime &&
                       (holdArrow->endTime == 0 || msecs < holdArrow->endTime);
        });
    bool isPressing = arrowHolders[direction]->getIsPressed();
    bool isActive = !SPRITE_isHidden(fakeHeads[i]->get());

    bool hidingNow = false;
    if (isHoldMode && isPressing) {
      if (!isActive)
        fakeHeads[i]->initialize(ArrowType::HOLD_FAKE_HEAD, direction);
    } else if (isActive) {
      hidingNow = true;
      SPRITE_hide(fakeHeads[i]->get());
    }

    ArrowState arrowState = fakeHeads[i]->tick(false, false);
    if (arrowState == ArrowState::OUT && !hidingNow)
      fakeHeads[i]->discard();
  }
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
}

SongScene::~SongScene() {
  EFFECT_turnOffBlend();
  arrowHolders.clear();
  fakeHeads.clear();
  Song_free(song);
}
