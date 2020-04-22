#include "SelectionScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "data/content/compiled/palette_selection.h"
#include "data/content/compiled/spr_arrows.h"
#include "gameplay/Key.h"
#include "gameplay/models/Song.h"
#include "scenes/SongScene.h"
#include "utils/BackgroundUtils.h"
#include "utils/SpriteUtils.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

const u32 ID_HIGHLIGHTER = 1;
const u32 ID_MAIN_BACKGROUND = 2;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 ARROW_HOLDERS = 4;

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

  for (u32 i = 0; i < ARROW_HOLDERS; i++)
    sprites.push_back(arrowHolders[i]->get());

  return sprites;
}

void SelectionScene::load() {
  BACKGROUND_enable(false, false, false, false);
  setUpPalettes();
  setUpBackground();
  setUpSprites();

  TextStream::instance().setText("Run to You", 15, 6);
}

void SelectionScene::tick(u16 keys) {
  for (auto& it : arrowHolders)
    it->tick();

  if (!hasStarted) {
    BACKGROUND_enable(true, true, true, false);
    highlighter->initialize();
    hasStarted = true;
  }

  if (KEY_DOWNLEFT(keys)) {
    highlighter->select(max(highlighter->getSelectedItem() - 1, 0));
    return;
  }
  if (KEY_DOWNRIGHT(keys)) {
    highlighter->select(min(highlighter->getSelectedItem() + 1, 3));
    return;
  }

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

void SelectionScene::setUpSprites() {
  for (u32 i = 0; i < ARROW_HOLDERS; i++) {
    auto arrowHolder = std::unique_ptr<ArrowHolder>{
        new ArrowHolder(static_cast<ArrowDirection>(i))};
    if (i == 0) {
      arrowHolder->get()->setData((void*)spr_arrowsTiles);
      arrowHolder->get()->setImageSize(sizeof(spr_arrowsTiles));
    }
    arrowHolders.push_back(std::move(arrowHolder));
  }
}

SelectionScene::~SelectionScene() {}
