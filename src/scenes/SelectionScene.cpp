#include "SelectionScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>
#include <tonc_input.h>

#include "assets.h"
#include "data/content/_compiled_sprites/palette_selection.h"
#include "gameplay/Key.h"
#include "gameplay/models/Song.h"
#include "gameplay/save/SaveFile.h"
#include "scenes/SettingsScene.h"
#include "scenes/SongScene.h"
#include "utils/SceneUtils.h"

extern "C" {
#include "player/fxes.h"
#include "player/player.h"
#include "utils/gbfs/gbfs.h"
}

#define CONFIRM_MESSAGE "Press    to start!"

const u32 ID_HIGHLIGHTER = 1;
const u32 ID_MAIN_BACKGROUND = 2;
const u32 INIT_FRAME = 2;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 PAGE_SIZE = 4;
const u32 SELECTOR_MARGIN = 3;
const u32 CENTER_X = 96;
const u32 CENTER_Y = 110;
const u32 MAX_DIFFICULTY = 2;
const u32 TEXT_COLOR = 0x7FFF;
const u32 TEXT_ROW = 13;
const int TEXT_SCROLL_NORMAL = -6;
const int TEXT_SCROLL_CONFIRMED = -10;
const u32 PIXEL_BLINK_LEVEL = 4;
const u32 CHANNEL_BADGE_X[] = {22, 84, 142, 203};
const u32 CHANNEL_BADGE_Y = 49;
const u32 GRADE_BADGE_X[] = {43, 103, 163, 222};
const u32 GRADE_BADGE_Y = 84;

static std::unique_ptr<Highlighter> highlighter{
    new Highlighter(ID_HIGHLIGHTER)};

SelectionScene::SelectionScene(std::shared_ptr<GBAEngine> engine,
                               const GBFS_FILE* fs)
    : Scene(engine) {
  this->fs = fs;
  library = std::unique_ptr<Library>{new Library(fs)};
}

std::vector<Background*> SelectionScene::backgrounds() {
  return {};
}

std::vector<Sprite*> SelectionScene::sprites() {
  std::vector<Sprite*> sprites;

  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    sprites.push_back(arrowSelectors[i]->get());

  for (u32 i = 0; i < PAGE_SIZE; i++)
    sprites.push_back(channelBadges[i]->get());

  for (u32 i = 0; i < PAGE_SIZE; i++)
    sprites.push_back(gradeBadges[i]->get());

  difficulty->render(&sprites);
  progress->render(&sprites);

  return sprites;
}

void SelectionScene::load() {
  SCENE_init();

  TextStream::instance().scroll(0, TEXT_SCROLL_NORMAL);

  difficulty = std::unique_ptr<Difficulty>{new Difficulty()};
  progress = std::unique_ptr<NumericProgress>{new NumericProgress()};
  pixelBlink = std::unique_ptr<PixelBlink>(new PixelBlink(PIXEL_BLINK_LEVEL));

  auto level = SAVEFILE_read8(SRAM->memory.difficultyLevel);
  difficulty->setValue(static_cast<DifficultyLevel>(level));

  setUpSpritesPalette();
  setUpArrows();
  setUpChannelBadges();
  setUpGradeBadges();
  setUpPager();
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
    highlighter->initialize(selected);
    init++;
  }

  pixelBlink->tick();
  for (auto& it : arrowSelectors)
    it->tick();

  processKeys(keys);
  processDifficultyChangeEvents();
  processSelectionChangeEvents();

  if (arrowSelectors[ArrowDirection::CENTER]->hasBeenPressedNow()) {
    if (confirmed)
      goToSong();
    else
      confirm();
  }

  if (keys & KEY_START) {
    player_stopAll();
    engine->transitionIntoScene(new SettingsScene(engine, fs),
                                new FadeOutScene(2));
  }

  blendAlpha = max(min(blendAlpha + (confirmed ? 1 : -1), MAX_BLEND),
                   HIGHLIGHTER_OPACITY);
  EFFECT_setBlendAlpha(blendAlpha);
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

  backgroundPalette =
      BACKGROUND_loadPaletteFile(fs, backgroundPaletteFile.c_str());
  backgroundPalette->persist();
  bg = BACKGROUND_loadBackgroundFiles(fs, backgroundTilesFile.c_str(),
                                      backgroundMapFile.c_str(),
                                      ID_MAIN_BACKGROUND);
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
  bg->setMosaic(true);
  bg->persist();

  TextStream::instance().setFontColor(TEXT_COLOR);

  loadChannels();
  loadProgress();
}

void SelectionScene::setUpArrows() {
  for (u32 i = 0; i < ARROWS_TOTAL; i++) {
    auto direction = static_cast<ArrowDirection>(i);
    arrowSelectors.push_back(std::unique_ptr<ArrowSelector>{
        new ArrowSelector(static_cast<ArrowDirection>(direction), direction > 0,
                          direction != ArrowDirection::CENTER)});
  }

  arrowSelectors[ArrowDirection::DOWNLEFT]->get()->moveTo(
      SELECTOR_MARGIN, GBA_SCREEN_HEIGHT - ARROW_SIZE - SELECTOR_MARGIN);
  arrowSelectors[ArrowDirection::UPLEFT]->get()->moveTo(SELECTOR_MARGIN,
                                                        SELECTOR_MARGIN);
  SPRITE_hide(arrowSelectors[2]->get());
  arrowSelectors[ArrowDirection::UPRIGHT]->get()->moveTo(
      GBA_SCREEN_WIDTH - ARROW_SIZE - SELECTOR_MARGIN, SELECTOR_MARGIN);
  arrowSelectors[ArrowDirection::DOWNRIGHT]->get()->moveTo(
      GBA_SCREEN_WIDTH - ARROW_SIZE - SELECTOR_MARGIN,
      GBA_SCREEN_HEIGHT - ARROW_SIZE - SELECTOR_MARGIN);
}

void SelectionScene::setUpChannelBadges() {
  for (u32 i = 0; i < PAGE_SIZE; i++) {
    channelBadges.push_back(std::unique_ptr<ChannelBadge>{
        new ChannelBadge(CHANNEL_BADGE_X[i], CHANNEL_BADGE_Y, i > 0)});
  }
}

void SelectionScene::setUpGradeBadges() {
  for (u32 i = 0; i < PAGE_SIZE; i++) {
    gradeBadges.push_back(std::unique_ptr<GradeBadge>{
        new GradeBadge(GRADE_BADGE_X[i], GRADE_BADGE_Y, i > 0)});
    gradeBadges[i]->get()->setPriority(ID_MAIN_BACKGROUND);
  }
}

void SelectionScene::setUpPager() {
  count = library->getCount();

  page = SAVEFILE_read8(SRAM->memory.pageIndex);
  selected = SAVEFILE_read8(SRAM->memory.songIndex);

  setPage(page, 0);
  updateSelection();
}

void SelectionScene::goToSong() {
  player_stopAll();
  confirmed = false;

  Song* song = Song_parse(fs, getSelectedSong(), true);
  Chart* chart = Song_findChartByDifficultyLevel(song, difficulty->getValue());
  SAVEFILE_write8(SRAM->state.pixelate, song->channel == Channel::ORIGINAL);

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
  arrowSelectors[ArrowDirection::DOWNLEFT]->setIsPressed(KEY_DOWNLEFT(keys));
  arrowSelectors[ArrowDirection::UPLEFT]->setIsPressed(KEY_UPLEFT(keys));
  arrowSelectors[ArrowDirection::CENTER]->setIsPressed(KEY_CENTER(keys));
  arrowSelectors[ArrowDirection::UPRIGHT]->setIsPressed(KEY_UPRIGHT(keys));
  arrowSelectors[ArrowDirection::DOWNRIGHT]->setIsPressed(KEY_DOWNRIGHT(keys));
}

void SelectionScene::processDifficultyChangeEvents() {
  if (onDifficultyChange(ArrowDirection::UPRIGHT,
                         static_cast<DifficultyLevel>(min(
                             (int)difficulty->getValue() + 1, MAX_DIFFICULTY))))
    return;

  onDifficultyChange(
      ArrowDirection::UPLEFT,
      static_cast<DifficultyLevel>(max((int)difficulty->getValue() - 1, 0)));
}

void SelectionScene::processSelectionChangeEvents() {
  if (onSelectionChange(ArrowDirection::DOWNRIGHT,
                        getSelectedSongIndex() == count - 1,
                        selected == PAGE_SIZE - 1, 1))
    return;

  onSelectionChange(ArrowDirection::DOWNLEFT, page == 0 && selected == 0,
                    selected == 0, -1);
}

bool SelectionScene::onDifficultyChange(ArrowDirection selector,
                                        DifficultyLevel newValue) {
  if (arrowSelectors[selector]->hasBeenPressedNow()) {
    unconfirm();
    fxes_play(SOUND_STEP);

    if (newValue == difficulty->getValue())
      return true;

    SAVEFILE_write8(SRAM->memory.difficultyLevel, newValue);

    difficulty->setValue(newValue);
    loadProgress();
    pixelBlink->blink();
    return true;
  }

  return false;
}

bool SelectionScene::onSelectionChange(ArrowDirection selector,
                                       bool isOnListEdge,
                                       bool isOnPageEdge,
                                       int direction) {
  if (arrowSelectors[selector]->shouldFireEvent()) {
    unconfirm();
    if (isOnListEdge) {
      if (arrowSelectors[selector]->hasBeenPressedNow())
        fxes_play(SOUND_STEP);
      return true;
    }

    player_stopAll();

    if (isOnPageEdge) {
      setPage(page + direction, direction);
      player_play(SOUND_STEP);
    } else {
      selected += direction;
      updateSelection();
      highlighter->select(selected);
      pixelBlink->blink();
      fxes_play(SOUND_STEP);
    }

    return true;
  }

  return false;
}

void SelectionScene::updateSelection() {
  Song* song = Song_parse(fs, getSelectedSong(), false);

  setNames(song->title, song->artist);
  player_play(song->audioPath.c_str());
  player_seek(song->sampleStart);
  Song_free(song);

  SAVEFILE_write8(SRAM->memory.pageIndex, page);
  SAVEFILE_write8(SRAM->memory.songIndex, selected);
}

void SelectionScene::confirm() {
  player_stop();
  fxes_play(SOUND_STEP);
  confirmed = true;
  arrowSelectors[ArrowDirection::CENTER]->get()->moveTo(CENTER_X, CENTER_Y);
  TextStream::instance().scroll(0, TEXT_SCROLL_CONFIRMED);
  TextStream::instance().clear();
  SCENE_write(CONFIRM_MESSAGE, TEXT_ROW);
}

void SelectionScene::unconfirm() {
  if (confirmed) {
    SPRITE_hide(arrowSelectors[ArrowDirection::CENTER]->get());
    TextStream::instance().scroll(0, TEXT_SCROLL_NORMAL);
    updateSelection();
  }
  confirmed = false;
}

void SelectionScene::setPage(u32 page, int direction) {
  this->page = page;

  songs.clear();
  songs = library->getSongs(page * PAGE_SIZE, PAGE_SIZE);

  if (direction == 0)
    setUpBackground();
  else {
    this->selected = direction < 0 ? PAGE_SIZE - 1 : 0;
    highlighter->select(selected);
    pixelBlink->blinkAndThen([this]() {
      setUpBackground();
      updateSelection();
    });
  }
}

void SelectionScene::loadChannels() {
  for (u32 i = 0; i < songs.size(); i++) {
    auto channel = Song_getChannel(fs, songs[i].get());
    channelBadges[i]->setType(channel);
  }

  for (u32 i = songs.size(); i < PAGE_SIZE; i++)
    channelBadges[i]->hide();
}

void SelectionScene::loadProgress() {
  auto difficultyLevel = difficulty->getValue();
  auto completedSongs =
      SAVEFILE_read32(SRAM->progress[difficultyLevel].completedSongs);

  progress->setValue(completedSongs, count);

  for (u32 i = 0; i < PAGE_SIZE; i++) {
    auto grade = SAVEFILE_getGradeOf(page * PAGE_SIZE + i, difficultyLevel);
    gradeBadges[i]->setType(grade);
  }
}

void SelectionScene::setNames(std::string title, std::string artist) {
  TextStream::instance().clear();
  SCENE_write(title, TEXT_ROW);
  TextStream::instance().setText("- " + artist + " -", TEXT_ROW + 1,
                                 TEXT_MIDDLE_COL - (artist.length() + 4) / 2);
}
