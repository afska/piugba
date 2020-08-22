#include "SelectionScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>
#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <tonc_input.h>

#include "SettingsScene.h"
#include "SongScene.h"
#include "assets.h"
#include "data/content/_compiled_sprites/palette_selection.h"
#include "gameplay/Key.h"
#include "gameplay/models/Song.h"
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
const u32 SELECTOR_MARGIN = 3;
const u32 CENTER_X = 96;
const u32 CENTER_Y = 110;
const u32 MAX_DIFFICULTY = 2;
const u32 TEXT_COLOR = 0x7FFF;
const u32 TEXT_ROW = 13;
const int TEXT_SCROLL_NORMAL = -6;
const int TEXT_SCROLL_CONFIRMED = -10;
const u32 PIXEL_BLINK_LEVEL = 4;
const u32 CHANNEL_BADGE_X[] = {22, 82, 142, 202};
const u32 CHANNEL_BADGE_Y = 49;
const u32 GRADE_BADGE_X[] = {43, 103, 163, 222};
const u32 GRADE_BADGE_Y = 84;
const u32 LOCK_X[] = {22, 82, 142, 202};
const u32 LOCK_Y = 71;
const u32 NUMERIC_LEVEL_BADGE_X = 104;
const u32 NUMERIC_LEVEL_BADGE_Y = 19;
const u32 NUMERIC_LEVEL_ROW = 3;
const u32 NUMERIC_LEVEL_BADGE_OFFSET_Y = 43;
const u32 NUMERIC_LEVEL_BADGE_OFFSET_ROW = 5;

// TODO: Select last unlocked arcade song
// TODO: Calculate available songs

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

  for (auto& it : arrowSelectors)
    sprites.push_back(it->get());

  for (auto& it : channelBadges)
    sprites.push_back(it->get());

  for (auto& it : gradeBadges)
    sprites.push_back(it->get());

  for (auto& it : locks)
    sprites.push_back(it->get());

  difficulty->render(&sprites);
  sprites.push_back(multiplier->get());
  progress->render(&sprites);

  if (getGameMode() == GameMode::ARCADE)
    sprites.push_back(numericLevelBadge->get());

  return sprites;
}

void SelectionScene::load() {
  SCENE_init();

  TextStream::instance().scroll(0, TEXT_SCROLL_NORMAL);

  difficulty = std::unique_ptr<Difficulty>{new Difficulty()};
  multiplier = std::unique_ptr<Multiplier>{
      new Multiplier(SAVEFILE_read8(SRAM->memory.multiplier))};
  progress = std::unique_ptr<NumericProgress>{new NumericProgress()};
  pixelBlink = std::unique_ptr<PixelBlink>(new PixelBlink(PIXEL_BLINK_LEVEL));

  if (getGameMode() == GameMode::ARCADE) {
    numericLevelBadge = std::unique_ptr<Button>{
        new Button(ButtonType::LEVEL_METER, NUMERIC_LEVEL_BADGE_X,
                   NUMERIC_LEVEL_BADGE_Y, false)};
    numericLevelBadge->get()->setPriority(ID_HIGHLIGHTER);

    difficulty->setValue(DifficultyLevel::NUMERIC);
    SPRITE_hide(multiplier->get());
  } else {
    auto level = SAVEFILE_read8(SRAM->memory.difficultyLevel);
    difficulty->setValue(static_cast<DifficultyLevel>(level));
  }

  setUpSpritesPalette();
  setUpArrows();
  setUpChannelBadges();
  setUpGradeBadges();
  setUpLocks();
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
  multiplier->tick();

  processKeys(keys);
  processDifficultyChangeEvents();
  processSelectionChangeEvents();
  processMultiplierChangeEvents();
  processConfirmEvents();
  processMenuEvents(keys);

  blendAlpha = max(min(blendAlpha + (confirmed ? 1 : -1), MAX_OPACITY),
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
  for (u32 i = 0; i < PAGE_SIZE; i++)
    channelBadges.push_back(std::unique_ptr<ChannelBadge>{
        new ChannelBadge(CHANNEL_BADGE_X[i], CHANNEL_BADGE_Y, i > 0)});
}

void SelectionScene::setUpGradeBadges() {
  for (u32 i = 0; i < PAGE_SIZE; i++) {
    gradeBadges.push_back(std::unique_ptr<GradeBadge>{
        new GradeBadge(GRADE_BADGE_X[i], GRADE_BADGE_Y, i > 0)});
    gradeBadges[i]->get()->setPriority(ID_MAIN_BACKGROUND);
  }
}

void SelectionScene::setUpLocks() {
  for (u32 i = 0; i < PAGE_SIZE; i++)
    locks.push_back(std::unique_ptr<Lock>{new Lock(LOCK_X[i], LOCK_Y, i > 0)});
}

void SelectionScene::setUpPager() {
  count = SAVEFILE_getLibrarySize();
  scrollTo(SAVEFILE_read8(SRAM->memory.pageIndex),
           SAVEFILE_read8(SRAM->memory.songIndex));
}

void SelectionScene::scrollTo(u32 page, u32 selected) {
  this->page = page;
  this->selected = selected;

  setPage(page, 0);
  updateSelection();
}

void SelectionScene::goToSong() {
  player_stopAll();
  confirmed = false;

  Song* song = SONG_parse(fs, getSelectedSong(), true);
  Chart* chart =
      getGameMode() == GameMode::ARCADE
          ? SONG_findChartByNumericLevel(song, getSelectedNumericLevel())
          : SONG_findChartByDifficultyLevel(song, difficulty->getValue());

  STATE_reset();
  GameState.isBoss = song->channel == Channel::BOSS;

  engine->transitionIntoScene(new SongScene(engine, fs, song, chart),
                              new FadeOutScene(4));
}

void SelectionScene::processKeys(u16 keys) {
  arrowSelectors[ArrowDirection::DOWNLEFT]->setIsPressed(KEY_DOWNLEFT(keys));
  arrowSelectors[ArrowDirection::UPLEFT]->setIsPressed(KEY_UPLEFT(keys));
  arrowSelectors[ArrowDirection::CENTER]->setIsPressed(KEY_CENTER(keys));
  arrowSelectors[ArrowDirection::UPRIGHT]->setIsPressed(KEY_UPRIGHT(keys));
  arrowSelectors[ArrowDirection::DOWNRIGHT]->setIsPressed(KEY_DOWNRIGHT(keys));
  multiplier->setIsPressed(keys & KEY_SELECT);
}

void SelectionScene::processDifficultyChangeEvents() {
  if (getGameMode() == GameMode::ARCADE) {
    auto level = getSelectedNumericLevel();
    auto currentLevelIndex = 0;
    for (u32 i = 0; i < numericLevels.size(); i++)
      if (numericLevels[i] == level) {
        currentLevelIndex = i;
        break;
      }
    auto previousIndex = max(currentLevelIndex - 1, 0);
    auto nextIndex = min(currentLevelIndex + 1, numericLevels.size() - 1);

    if (onNumericLevelChange(ArrowDirection::UPRIGHT, numericLevels[nextIndex]))
      return;

    onNumericLevelChange(ArrowDirection::UPLEFT, numericLevels[previousIndex]);
  } else {
    if (onDifficultyLevelChange(
            ArrowDirection::UPRIGHT,
            static_cast<DifficultyLevel>(
                min((int)difficulty->getValue() + 1, MAX_DIFFICULTY))))
      return;

    onDifficultyLevelChange(
        ArrowDirection::UPLEFT,
        static_cast<DifficultyLevel>(max((int)difficulty->getValue() - 1, 0)));
  }
}

void SelectionScene::processSelectionChangeEvents() {
  auto isOnListEdge = getSelectedSongIndex() == getLastUnlockedSongIndex();
  if (IGNORE_LOCKS)
    isOnListEdge = getSelectedSongIndex() == count - 1;

  if (onSelectionChange(ArrowDirection::DOWNRIGHT, isOnListEdge,
                        selected == PAGE_SIZE - 1, 1))
    return;

  onSelectionChange(ArrowDirection::DOWNLEFT, page == 0 && selected == 0,
                    selected == 0, -1);
}

void SelectionScene::processMultiplierChangeEvents() {
  if (getGameMode() == GameMode::ARCADE)
    return;

  if (multiplier->hasBeenPressedNow()) {
    fxes_playSolo(SOUND_MOD);
    SAVEFILE_write8(SRAM->memory.multiplier, multiplier->change());
  }
}

void SelectionScene::processConfirmEvents() {
  if (arrowSelectors[ArrowDirection::CENTER]->hasBeenPressedNow()) {
    if (confirmed)
      goToSong();
    else
      confirm();
  }
}

void SelectionScene::processMenuEvents(u16 keys) {
  if (keys & KEY_START) {
    player_stopAll();
    engine->transitionIntoScene(new SettingsScene(engine, fs),
                                new FadeOutScene(4));
  }
}

bool SelectionScene::onDifficultyLevelChange(ArrowDirection selector,
                                             DifficultyLevel newValue) {
  if (arrowSelectors[selector]->hasBeenPressedNow()) {
    unconfirm();
    fxes_playSolo(SOUND_STEP);

    if (newValue == difficulty->getValue())
      return true;

    SAVEFILE_write8(SRAM->memory.difficultyLevel, newValue);

    difficulty->setValue(newValue);
    pixelBlink->blink();

    u32 lastUnlockedSongIndex = getLastUnlockedSongIndex();
    if (lastUnlockedSongIndex >= getSelectedSongIndex())
      loadProgress();
    else {
      player_stopAll();
      scrollTo(Div(lastUnlockedSongIndex, PAGE_SIZE),
               DivMod(lastUnlockedSongIndex, PAGE_SIZE));
    }

    return true;
  }

  return false;
}

bool SelectionScene::onNumericLevelChange(ArrowDirection selector,
                                          u8 newValue) {
  if (arrowSelectors[selector]->hasBeenPressedNow()) {
    unconfirm();
    fxes_playSolo(SOUND_STEP);

    if (newValue == getSelectedNumericLevel())
      return true;

    SAVEFILE_write8(SRAM->memory.numericLevel, newValue);
    updateSelection(false);
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
        fxes_playSolo(SOUND_STEP);
      return true;
    }

    player_stopAll();

    if (isOnPageEdge) {
      setPage(page + direction, direction);
      player_play(SOUND_STEP);
    } else {
      selected += direction;
      updateSelection();
      pixelBlink->blink();
      fxes_play(SOUND_STEP);
    }

    return true;
  }

  return false;
}

void SelectionScene::updateSelection(bool withMusic) {
  Song* song = SONG_parse(fs, getSelectedSong(), false);

  numericLevels.clear();
  for (u32 i = 0; i < song->chartCount; i++)
    numericLevels.push_back(song->charts[i].level);
  setClosestNumericLevel();
  setNames(song->title, song->artist);
  printNumericLevel();
  if (withMusic) {
    player_play(song->audioPath.c_str());
    player_seek(song->sampleStart);
  }
  SONG_free(song);

  SAVEFILE_write8(SRAM->memory.pageIndex, page);
  SAVEFILE_write8(SRAM->memory.songIndex, selected);
  highlighter->select(selected);
}

void SelectionScene::confirm() {
  if (getGameMode() == GameMode::ARCADE)
    numericLevelBadge->get()->moveTo(
        NUMERIC_LEVEL_BADGE_X,
        NUMERIC_LEVEL_BADGE_Y + NUMERIC_LEVEL_BADGE_OFFSET_Y);

  fxes_playSolo(SOUND_STEP);
  confirmed = true;
  arrowSelectors[ArrowDirection::CENTER]->get()->moveTo(CENTER_X, CENTER_Y);
  TextStream::instance().scroll(0, TEXT_SCROLL_CONFIRMED);
  TextStream::instance().clear();
  printNumericLevel(NUMERIC_LEVEL_BADGE_OFFSET_ROW);
  SCENE_write(CONFIRM_MESSAGE, TEXT_ROW);
}

void SelectionScene::unconfirm() {
  if (confirmed) {
    if (getGameMode() == GameMode::ARCADE)
      numericLevelBadge->get()->moveTo(NUMERIC_LEVEL_BADGE_X,
                                       NUMERIC_LEVEL_BADGE_Y);

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
    auto channel = SONG_getChannel(fs, songs[i].get());
    channelBadges[i]->setType(channel);
  }

  for (u32 i = songs.size(); i < PAGE_SIZE; i++)
    channelBadges[i]->hide();
}

void SelectionScene::loadProgress() {
  progress->setValue(getCompletedSongs(), count);

  for (u32 i = 0; i < PAGE_SIZE; i++) {
    auto songId = page * PAGE_SIZE + i;
    auto grade = SAVEFILE_getGradeOf(songId, difficulty->getValue());
    gradeBadges[i]->setType(grade);
    locks[i]->setVisible(songId > getLastUnlockedSongIndex() &&
                         songId <= count - 1);
  }
}

void SelectionScene::setNames(std::string title, std::string artist) {
  TextStream::instance().clear();
  SCENE_write(title, TEXT_ROW);
  TextStream::instance().setText("- " + artist + " -", TEXT_ROW + 1,
                                 TEXT_MIDDLE_COL - (artist.length() + 4) / 2);
}

void SelectionScene::printNumericLevel(s8 offset) {
  if (getGameMode() != GameMode::ARCADE)
    return;

  auto levelText = std::to_string(getSelectedNumericLevel());
  if (levelText.size() == 1)
    levelText = "0" + levelText;
  SCENE_write(levelText, NUMERIC_LEVEL_ROW + offset);
}

SelectionScene::~SelectionScene() {
  songs.clear();
  arrowSelectors.clear();
  channelBadges.clear();
  gradeBadges.clear();
  locks.clear();
}
