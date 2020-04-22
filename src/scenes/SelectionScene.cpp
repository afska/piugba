#include "SelectionScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "data/bg_selectionmask.h"
#include "gameplay/models/Song.h"
#include "scenes/SongScene.h"
#include "utils/BackgroundUtils.h"
#include "utils/EffectUtils.h"
#include "utils/SpriteUtils.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

const u32 ID_HIGHLIGHTER = 1;
const u32 ID_MAIN_BACKGROUND = 2;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;

static const GBFS_FILE* fs = find_first_gbfs_file(0);
static std::unique_ptr<Library> library{new Library(fs)};
static std::unique_ptr<Highlighter> highlighter{new Highlighter()};

SelectionScene::SelectionScene(std::shared_ptr<GBAEngine> engine)
    : Scene(engine) {}

std::vector<Background*> SelectionScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> SelectionScene::sprites() {
  std::vector<Sprite*> sprites;

  return sprites;
}

void SelectionScene::load() {
  BACKGROUND_enable(false, false, false, false);
  setUpPalettes();
  setUpBackground();

  TextStream::instance().setText("Run to You", 15, 6);
}

void SelectionScene::tick(u16 keys) {
  if (!hasStarted) {
    hasStarted = true;
    BACKGROUND_enable(true, true, true, false);

    BACKGROUND_setup(ID_HIGHLIGHTER, BG_SELECTIONMASK_METADATA.TILES_BANK,
                     BG_SELECTIONMASK_METADATA.MAP_BANK);
    BACKGROUND_loadPalette(BG_SELECTIONMASK_PALETTE,
                           BG_SELECTIONMASK_METADATA.PALETTE_LENGTH,
                           BG_SELECTIONMASK_METADATA.PALETTE_START_INDEX);
    BACKGROUND_loadTiles(BG_SELECTIONMASK_TILES[1],
                         BG_SELECTIONMASK_METADATA.TILES_LENGTH,
                         BG_SELECTIONMASK_METADATA.TILES_BANK,
                         BG_SELECTIONMASK_METADATA.TILES_START_INDEX);
    BACKGROUND_loadMap(BG_SELECTIONMASK_MAP,
                       BG_SELECTIONMASK_METADATA.MAP_TOTAL_TILES,
                       BG_SELECTIONMASK_METADATA.MAP_BANK,
                       BG_SELECTIONMASK_METADATA.MAP_START_INDEX,
                       BG_SELECTIONMASK_METADATA.MAP_END_INDEX,
                       BG_SELECTIONMASK_METADATA.TILES_START_INDEX);

    pal_bg_mem[BG_SELECTIONMASK_METADATA.PALETTE_START_INDEX] = 0;
    pal_bg_mem[BG_SELECTIONMASK_METADATA.PALETTE_START_INDEX + 1] = 0;
    pal_bg_mem[BG_SELECTIONMASK_METADATA.PALETTE_START_INDEX + 2] = 0;
    pal_bg_mem[BG_SELECTIONMASK_METADATA.PALETTE_START_INDEX + 3] = 0;

    EFFECT_setUpBlend(BLD_BG1, BLD_BG2);
    EFFECT_setBlendAlpha(24);
  }

  i += inc;
  LOG(i);
  EFFECT_setBlendAlpha(i);
  if (i == MIN_BLEND || i == MAX_BLEND)
    inc *= -1;

  if (keys & KEY_ANY && !engine->isTransitioning()) {
    char* name;
    u8 level;

    if (keys & KEY_SELECT) {
      name = (char*)"With my Lover";
      level = 10;
    } else if (keys & KEY_UP) {
      name = (char*)"Witch Doctor";
      level = 16;
    } else if (keys & KEY_B) {
      name = (char*)"Beat of the War 2";
      level = 16;
    } else if (keys & KEY_A) {
      name = (char*)"Tepris";
      level = 16;
    } else if (keys & KEY_L) {
      name = (char*)"Beethoven Virus";
      level = 7;
    } else if (keys & KEY_R) {
      name = (char*)"Beethoven Virus";
      level = 13;
    } else if (keys & KEY_LEFT) {
      name = (char*)"Run to You";
      level = 5;
    } else if (keys & KEY_RIGHT) {
      name = (char*)"414-Run to You";
      level = 12;
    } else if (keys & KEY_START) {
      name = (char*)"Extravaganza";
      level = 11;
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

SelectionScene::~SelectionScene() {}
