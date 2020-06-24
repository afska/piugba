#include "SelectionScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>
#include <tonc_input.h>

#include "assets.h"
#include "data/content/_compiled_sprites/palette_selection.h"
#include "gameplay/Key.h"
#include "gameplay/models/Song.h"
#include "scenes/SongScene.h"
#include "ui/Highlighter.h"
#include "utils/BackgroundUtils.h"
#include "utils/EffectUtils.h"
#include "utils/SpriteUtils.h"

extern "C" {
#include "player/fxes.h"
#include "player/player.h"
#include "utils/gbfs/gbfs.h"
}

const u32 ID_HIGHLIGHTER = 1;
const u32 ID_MAIN_BACKGROUND = 2;
const u32 INIT_FRAME = 2;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 PAGE_SIZE = 4;
const u32 ARROW_SELECTORS = 4;
const u32 SELECTOR_PREVIOUS_DIFFICULTY = 1;
const u32 SELECTOR_NEXT_DIFFICULTY = 2;
const u32 SELECTOR_PREVIOUS_SONG = 0;
const u32 SELECTOR_NEXT_SONG = 3;
const u32 SELECTOR_MARGIN = 3;
const u32 TEXT_ROW = 13;
const u32 TEXT_MIDDLE_COL = 12;
const u32 MAX_DIFFICULTY = 2;
const u32 TEXT_COLOR = 0x7FFF;
const u32 BLINK_LEVEL = 4;

static const GBFS_FILE* fs = find_first_gbfs_file(0);
static std::unique_ptr<Library> library{new Library(fs)};
static std::unique_ptr<Highlighter> highlighter{
    new Highlighter(ID_HIGHLIGHTER)};

SelectionScene::SelectionScene(std::shared_ptr<GBAEngine> engine)
    : Scene(engine) {}

std::vector<Background*> SelectionScene::backgrounds() {
  return {};
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

  difficulty = std::unique_ptr<Difficulty>{new Difficulty()};
  progress = std::unique_ptr<NumericProgress>{new NumericProgress()};
  pixelBlink = std::unique_ptr<PixelBlink>(new PixelBlink(BLINK_LEVEL));
  selectInput = std::unique_ptr<InputHandler>(new InputHandler());
  setUpPager();

  setUpSpritesPalette();
  setUpArrows();
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
  processDifficultyChange();
  processSelectionChange();

  if (selectInput->hasBeenPressedNow())
    goToSong();
}

void SelectionScene::setUpSpritesPalette() {
  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>(new ForegroundPaletteManager(
          palette_selectionPal, sizeof(palette_selectionPal)));
}

void SelectionScene::setUpBackground() {
  auto backgroundFile = "_sel_" + std::to_string(getPageStart());
  auto backgroundPaletteFile = backgroundFile + BACKGROUND_PALETTE_EXTENSION;
  auto backgroundTilesFile = backgroundFile + BACKGROUND_TILES_EXTENSION;
  auto backgroundMapFile = backgroundFile + BACKGROUND_MAP_EXTENSION;

  u32 backgroundPaletteLength;
  auto backgroundPaletteData = (COLOR*)gbfs_get_obj(
      fs, backgroundPaletteFile.c_str(), &backgroundPaletteLength);
  backgroundPalette =
      std::unique_ptr<BackgroundPaletteManager>(new BackgroundPaletteManager(
          backgroundPaletteData, backgroundPaletteLength));
  backgroundPalette->persist();

  u32 backgroundTilesLength, backgroundMapLength;
  auto backgroundTilesData =
      gbfs_get_obj(fs, backgroundTilesFile.c_str(), &backgroundTilesLength);
  auto backgroundMapData =
      gbfs_get_obj(fs, backgroundMapFile.c_str(), &backgroundMapLength);
  bg = std::unique_ptr<Background>(new Background(
      ID_MAIN_BACKGROUND, backgroundTilesData, backgroundTilesLength,
      backgroundMapData, backgroundMapLength));
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
  bg->setMosaic(true);
  bg->persist();

  TextStream::instance().setFontColor(TEXT_COLOR);
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

void SelectionScene::setUpPager() {
  count = library->getCount();
  setPage(0, 0);
  updatePage();
}

void SelectionScene::goToSong() {
  player_stop();
  fxes_stop();

  Song* song = Song_parse(fs, getSelectedSong(), true);
  Chart* chart = Song_findChartByDifficultyLevel(song, difficulty->getValue());

  engine->transitionIntoScene(new SongScene(engine, fs, song, chart),
                              new FadeOutScene(2));
}

SongFile* SelectionScene::getSelectedSong() {
  return songs[selected].get();
}

u32 SelectionScene::getSelectedSongIndex() {
  return page * PAGE_SIZE + selected;
}

u32 SelectionScene::getPageStart() {
  return page * PAGE_SIZE;
}

void SelectionScene::processKeys(u16 keys) {
  arrowSelectors[0]->setIsPressed(KEY_DOWNLEFT(keys));
  arrowSelectors[1]->setIsPressed(KEY_UPLEFT(keys));
  arrowSelectors[2]->setIsPressed(KEY_UPRIGHT(keys));
  arrowSelectors[3]->setIsPressed(KEY_DOWNRIGHT(keys));
  selectInput->setIsPressed(KEY_CENTER(keys));
}

void SelectionScene::processDifficultyChange() {
  if (arrowSelectors[SELECTOR_PREVIOUS_DIFFICULTY]->hasBeenPressedNow()) {
    fxes_play(SOUND_MOVE);

    auto newValue =
        static_cast<DifficultyLevel>(max((int)difficulty->getValue() - 1, 0));
    if (newValue == difficulty->getValue())
      return;

    difficulty->setValue(newValue);
    pixelBlink->blink();
    return;
  }

  if (arrowSelectors[SELECTOR_NEXT_DIFFICULTY]->hasBeenPressedNow()) {
    fxes_play(SOUND_MOVE);

    auto newValue = static_cast<DifficultyLevel>(
        min((int)difficulty->getValue() + 1, MAX_DIFFICULTY));
    if (newValue == difficulty->getValue())
      return;

    difficulty->setValue(newValue);
    pixelBlink->blink();
    return;
  }
}

void SelectionScene::processSelectionChange() {
  if (arrowSelectors[SELECTOR_NEXT_SONG]->hasBeenPressedNow()) {
    player_stop();
    fxes_play(SOUND_MOVE);

    if (getSelectedSongIndex() == count - 1)
      return;

    if (selected == PAGE_SIZE - 1)
      setPage(page + 1, 1);
    else {
      selected++;
      updatePage();
      highlighter->select(selected);
      pixelBlink->blink();
    }

    return;
  }

  if (arrowSelectors[SELECTOR_PREVIOUS_SONG]->hasBeenPressedNow()) {
    player_stop();
    fxes_play(SOUND_MOVE);

    if (page == 0 && selected == 0)
      return;

    if (selected == 0)
      setPage(page - 1, -1);
    else {
      selected--;
      updatePage();
      highlighter->select(selected);
      pixelBlink->blink();
    }

    return;
  }
}

void SelectionScene::updatePage() {
  Song* song = Song_parse(fs, getSelectedSong(), false);
  setNames(song->title, song->artist);
  player_play(song->audioPath.c_str());
  Song_free(song);
}

void SelectionScene::setPage(u32 page, int direction) {
  progress->setValue(0, count);  // TODO: Implement progress

  this->page = page;
  this->selected = direction < 0 ? PAGE_SIZE - 1 : 0;
  songs.clear();
  songs = library->getSongs(page * PAGE_SIZE, PAGE_SIZE);

  highlighter->select(selected);
  if (direction == 0)
    setUpBackground();
  else
    pixelBlink->blinkAndThen([this]() {
      setUpBackground();
      updatePage();
    });
}

void SelectionScene::setNames(std::string title, std::string artist) {
  TextStream::instance().clear();
  TextStream::instance().setText(title, TEXT_ROW,
                                 TEXT_MIDDLE_COL - title.length() / 2);
  TextStream::instance().setText("- " + artist + " -", TEXT_ROW + 1,
                                 TEXT_MIDDLE_COL - (artist.length() + 4) / 2);
}

SelectionScene::~SelectionScene() {
  EFFECT_turnOffBlend();
  EFFECT_turnOffMosaic();
}
