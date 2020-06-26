#include "SongScene.h"

#include <libgba-sprite-engine/effects/fade_out_scene.h>
#include <libgba-sprite-engine/gba/tonc_math.h>
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

const u32 DARKENER_ID = 0;
const u32 DARKENER_PRIORITY = 2;
const u32 MAIN_BACKGROUND_ID = 1;
const u32 MAIN_BACKGROUND_PRIORITY = 3;
const u32 ARROW_POOL_SIZE = 50;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 24;
const u32 PIXEL_BLINK_LEVEL = 2;
const u32 ALPHA_BLINK_TIME = 6;

static std::unique_ptr<Darkener> darkener{
    new Darkener(DARKENER_ID, DARKENER_PRIORITY)};

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

  for (u32 i = 0; i < ARROWS_TOTAL; i++) {
    fakeHeads[i]->index = sprites.size();
    sprites.push_back(fakeHeads[i]->get());
  }

  arrowPool->forEach([&sprites](Arrow* it) {
    it->index = sprites.size();
    sprites.push_back(it->get());
  });

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

  pixelBlink = std::unique_ptr<PixelBlink>(new PixelBlink(PIXEL_BLINK_LEVEL));
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
  chartReader = std::unique_ptr<ChartReader>(
      new ChartReader(chart, arrowPool.get(), judge.get(), pixelBlink.get()));

  speedUpInput = std::unique_ptr<InputHandler>(new InputHandler());
  speedDownInput = std::unique_ptr<InputHandler>(new InputHandler());
}

void SongScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (!hasStarted) {
    IFNOTTEST {
      BACKGROUND_enable(true, true, false, false);
      darkener->initialize();
    }
    IFTEST { BACKGROUND_setColor(0, 127); }
    hasStarted = true;
  }

  u32 songMsecs = PlaybackState.msecs;

  if (PlaybackState.hasFinished || songMsecs >= song->lastMillisecond) {
    unload();
    engine->transitionIntoScene(new SelectionScene(engine),
                                new FadeOutScene(2));
    return;
  }

  updateArrowHolders();
  processKeys(keys);

  bool isNewBeat = chartReader->update((int)songMsecs);
  if (isNewBeat) {
    blinkFrame += ALPHA_BLINK_TIME;

    for (auto& arrowHolder : arrowHolders) {
      lifeBar->blink(foregroundPalette.get());
      if (!KEY_ANY_PRESSED(keys))
        arrowHolder->blink();
    }
  }

  blinkFrame = max(blinkFrame - 1, 0);
  EFFECT_setBlendAlpha(DARKENER_OPACITY - blinkFrame);

  pixelBlink->tick();
  updateFakeHeads();
  updateArrows();
  score->tick();
  lifeBar->tick(foregroundPalette.get());

  IFTIMINGTEST { chartReader->logDebugInfo<CHART_DEBUG>(); }
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
      MAIN_BACKGROUND_ID, backgroundTilesData, backgroundTilesLength,
      backgroundMapData, backgroundMapLength));
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
  bg->usePriority(MAIN_BACKGROUND_PRIORITY);
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
  std::array<Arrow*, ARROWS_TOTAL> nextArrows;
  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    nextArrows[i] = NULL;

  // update sprites
  arrowPool->forEachActive([&nextArrows, this](Arrow* arrow) {
    ArrowDirection direction = arrow->direction;
    bool isStopped = chartReader->isStopped();

    int newY = chartReader->getYFor(arrow);
    bool isPressing = arrowHolders[direction]->getIsPressed() && !isStopped;
    ArrowState arrowState = arrow->tick(newY, isPressing);

    if (arrowState == ArrowState::OUT) {
      judge->onOut(arrow);
      return;
    }

    bool canBeJudged =
        arrow->type == ArrowType::UNIQUE && !arrow->getIsPressed();
    if (canBeJudged && (nextArrows[direction] == NULL ||
                        arrow->timestamp < nextArrows[direction]->timestamp))
      nextArrows[direction] = arrow;
  });

  // judge key press events
  for (u32 i = 0; i < ARROWS_TOTAL; i++) {
    auto arrow = nextArrows[i];
    bool isStopped = chartReader->isStopped();
    if (arrow == NULL)
      continue;

    ArrowDirection direction = arrow->direction;
    bool canBeJudged = true;
    int judgementOffset = 0;

    if (isStopped) {
      bool hasJustStopped = chartReader->hasJustStopped();
      bool isAboutToResume = chartReader->isAboutToResume();

      canBeJudged = arrow->timestamp >= chartReader->getStopStart() &&
                    (hasJustStopped || isAboutToResume);
      judgementOffset = isAboutToResume ? -chartReader->getStopLength() : 0;

      if (chartReader->isStopJudgeable()) {
        canBeJudged = true;
        judgementOffset =
            -(chartReader->getMsecs() - chartReader->getStopStart());
      }
    }

    bool hasBeenPressedNow = arrowHolders[direction]->hasBeenPressedNow();
    if (canBeJudged && hasBeenPressedNow)
      judge->onPress(arrow, chartReader.get(), judgementOffset);
  }
}

void SongScene::updateFakeHeads() {
  for (u32 i = 0; i < ARROWS_TOTAL; i++) {
    auto direction = static_cast<ArrowDirection>(i);

    bool isHoldMode = chartReader->isHoldActive(direction);
    bool isPressing = arrowHolders[direction]->getIsPressed();
    bool isVisible = fakeHeads[i]->get()->enabled;

    if (isHoldMode && isPressing && !chartReader->isStopped()) {
      if (!isVisible) {
        fakeHeads[i]->initialize(ArrowType::HOLD_FAKE_HEAD, direction, 0,
                                 false);
        isVisible = true;
      }
    } else if (isVisible) {
      fakeHeads[i]->discard();
      isVisible = false;
    }

    if (isVisible)
      fakeHeads[i]->tick(0, false);
  }
}

void SongScene::processKeys(u16 keys) {
  arrowHolders[0]->setIsPressed(KEY_DOWNLEFT(keys));
  arrowHolders[1]->setIsPressed(KEY_UPLEFT(keys));
  arrowHolders[2]->setIsPressed(KEY_CENTER(keys));
  arrowHolders[3]->setIsPressed(KEY_UPRIGHT(keys));
  arrowHolders[4]->setIsPressed(KEY_DOWNRIGHT(keys));
  speedUpInput->setIsPressed(keys & KEY_START);
  speedDownInput->setIsPressed(keys & KEY_SELECT);

  if (speedUpInput->hasBeenPressedNow())
    chartReader->setMultiplier(chartReader->getMultiplier() + 1);

  if (speedDownInput->hasBeenPressedNow())
    chartReader->setMultiplier(chartReader->getMultiplier() - 1);

  IFSTRESSTEST {
    for (auto& arrowHolder : arrowHolders)
      if (arrowHolder->hasBeenPressedNow())
        arrowPool->create([&arrowHolder, this](Arrow* it) {
          it->initialize(ArrowType::UNIQUE, arrowHolder->direction,
                         chartReader->getMsecs() + chartReader->getArrowTime(),
                         false);
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
