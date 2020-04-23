#include "SelectionScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "data/content/compiled/palette_selection.h"
#include "gameplay/Key.h"
#include "gameplay/models/Song.h"
#include "scenes/SongScene.h"
#include "ui/Highlighter.h"
#include "utils/BackgroundUtils.h"
#include "utils/EffectUtils.h"
#include "utils/SpriteUtils.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

const u32 ID_HIGHLIGHTER = 1;
const u32 ID_MAIN_BACKGROUND = 2;
const u32 INIT_FRAME = 2;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 SONG_ITEMS = 4;
const u32 ARROW_SELECTORS = 4;
const u32 SELECTOR_PREVIOUS_DIFFICULTY = 1;
const u32 SELECTOR_NEXT_DIFFICULTY = 2;
const u32 SELECTOR_PREVIOUS_SONG = 0;
const u32 SELECTOR_NEXT_SONG = 3;
const u32 SELECTOR_MARGIN = 3;
const u32 TEXT_ROW = 13;
const u32 TEXT_MIDDLE_COL = 12;
const u32 MAX_DIFFICULTY = 2;

static const GBFS_FILE* fs = find_first_gbfs_file(0);
static std::unique_ptr<Library> library{new Library(fs)};
static std::unique_ptr<Highlighter> highlighter{
    new Highlighter(ID_HIGHLIGHTER)};

SelectionScene::SelectionScene(std::shared_ptr<GBAEngine> engine)
    : Scene(engine) {}

std::vector<Background*> SelectionScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> SelectionScene::sprites() {
  std::vector<Sprite*> sprites;

  for (u32 i = 0; i < ARROW_SELECTORS; i++)
    sprites.push_back(arrowSelectors[i]->get());

  difficulty->render(&sprites);
  progress->render(&sprites);

  return sprites;
}

void SelectionScene::load() {
  BACKGROUND_enable(false, false, false, false);
  SPRITE_disable();
  setUpPalettes();
  setUpBackground();
  setUpBlink();
  setUpArrows();
  difficulty = std::unique_ptr<Difficulty>{new Difficulty()};
  progress = std::unique_ptr<NumericProgress>{new NumericProgress()};
  progress->setValue(245, 800);

  TextStream::instance().setText("Run to You", TEXT_ROW,
                                 TEXT_MIDDLE_COL - strlen("Run to You") / 2);
}

void SelectionScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (init < INIT_FRAME) {
    init++;
    return;
  } else if (init == INIT_FRAME) {
    BACKGROUND_enable(true, true, true, false);
    SPRITE_enable();
    highlighter->initialize();
    init++;
  }

  pixelBlink->tick();
  for (auto& it : arrowSelectors)
    it->tick();

  processKeys(keys);

  if (arrowSelectors[SELECTOR_PREVIOUS_DIFFICULTY]->hasBeenPressedNow()) {
    difficulty->setValue(
        static_cast<DifficultyLevel>(max((int)difficulty->getValue() - 1, 0)));
    pixelBlink->blink();
    return;
  }

  if (arrowSelectors[SELECTOR_NEXT_DIFFICULTY]->hasBeenPressedNow()) {
    difficulty->setValue(static_cast<DifficultyLevel>(
        min((int)difficulty->getValue() + 1, MAX_DIFFICULTY)));
    pixelBlink->blink();
    return;
  }

  if (arrowSelectors[SELECTOR_PREVIOUS_SONG]->hasBeenPressedNow()) {
    highlighter->select(max(highlighter->getSelectedItem() - 1, 0));
    pixelBlink->blink();
    return;
  }

  if (arrowSelectors[SELECTOR_NEXT_SONG]->hasBeenPressedNow()) {
    highlighter->select(
        min(highlighter->getSelectedItem() + 1, SONG_ITEMS - 1));
    pixelBlink->blink();
    return;
  }

  if (keys & KEY_ANY && !engine->isTransitioning()) {
    char* name;
    u8 level;

    if (keys & KEY_RIGHT) {
      name = (char*)"404-Solitary";
      level = 18;
    } else {
      return;
    }

    Song* song = Song_parse(fs, SongFile(name));
    Chart* chart = Song_findChartByLevel(song, level);

    engine->transitionIntoScene(new SongScene(engine, fs, song, chart),
                                new FadeOutScene(2));
  }
}

void SelectionScene::setUpPalettes() {
  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>(new ForegroundPaletteManager(
          palette_selectionPal, sizeof(palette_selectionPal)));

  u32 backgroundPaletteLength;
  auto backgroundPaletteData =
      (COLOR*)gbfs_get_obj(fs, "output.pal.bin", &backgroundPaletteLength);
  backgroundPalette =
      std::unique_ptr<BackgroundPaletteManager>(new BackgroundPaletteManager(
          backgroundPaletteData, backgroundPaletteLength));
}

void SelectionScene::setUpBackground() {
  u32 backgroundTilesLength, backgroundMapLength;
  auto backgroundTilesData =
      gbfs_get_obj(fs, "output.img.bin", &backgroundTilesLength);
  auto backgroundMapData =
      gbfs_get_obj(fs, "output.map.bin", &backgroundMapLength);
  bg = std::unique_ptr<Background>(new Background(
      ID_MAIN_BACKGROUND, backgroundTilesData, backgroundTilesLength,
      backgroundMapData, backgroundMapLength));
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
}

void SelectionScene::setUpBlink() {
  pixelBlink = std::unique_ptr<PixelBlink>(new PixelBlink());
  bg->setMosaic(true);
}

void SelectionScene::setUpArrows() {
  arrowSelectors.push_back(std::unique_ptr<ArrowSelector>{new ArrowSelector(
      static_cast<ArrowDirection>(ArrowDirection::DOWNLEFT))});
  arrowSelectors.push_back(std::unique_ptr<ArrowSelector>{
      new ArrowSelector(static_cast<ArrowDirection>(ArrowDirection::UPLEFT))});
  arrowSelectors.push_back(std::unique_ptr<ArrowSelector>{
      new ArrowSelector(static_cast<ArrowDirection>(ArrowDirection::UPRIGHT))});
  arrowSelectors.push_back(std::unique_ptr<ArrowSelector>{new ArrowSelector(
      static_cast<ArrowDirection>(ArrowDirection::DOWNRIGHT))});

  arrowSelectors[0]->get()->moveTo(
      SELECTOR_MARGIN, GBA_SCREEN_HEIGHT - ARROW_SIZE - SELECTOR_MARGIN);
  arrowSelectors[1]->get()->moveTo(SELECTOR_MARGIN, SELECTOR_MARGIN);
  arrowSelectors[2]->get()->moveTo(
      GBA_SCREEN_WIDTH - ARROW_SIZE - SELECTOR_MARGIN, SELECTOR_MARGIN);
  arrowSelectors[3]->get()->moveTo(
      GBA_SCREEN_WIDTH - ARROW_SIZE - SELECTOR_MARGIN,
      GBA_SCREEN_HEIGHT - ARROW_SIZE - SELECTOR_MARGIN);
}

void SelectionScene::processKeys(u16 keys) {
  arrowSelectors[0]->setIsPressed(KEY_DOWNLEFT(keys));
  arrowSelectors[1]->setIsPressed(KEY_UPLEFT(keys));
  arrowSelectors[2]->setIsPressed(KEY_UPRIGHT(keys));
  arrowSelectors[3]->setIsPressed(KEY_DOWNRIGHT(keys));
}

SelectionScene::~SelectionScene() {
  EFFECT_turnOffBlend();
  EFFECT_turnOffMosaic();
}
