#include "SelectionScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "gameplay/models/Song.h"
#include "scenes/SongScene.h"
#include "utils/SpriteUtils.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

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
  engine->disableText();

  u32 backgroundPaletteLength;
  auto backgroundPaletteData =
      (COLOR*)gbfs_get_obj(fs, "output.pal.bin", &backgroundPaletteLength);
  backgroundPalette =
      std::unique_ptr<BackgroundPaletteManager>(new BackgroundPaletteManager(
          backgroundPaletteData, backgroundPaletteLength));

  u32 backgroundTilesLength, backgroundMapLength;
  auto backgroundTilesData =
      gbfs_get_obj(fs, "output.img.bin", &backgroundTilesLength);
  auto backgroundMapData =
      gbfs_get_obj(fs, "output.map.bin", &backgroundMapLength);
  bg = std::unique_ptr<Background>(
      new Background(1, backgroundTilesData, backgroundTilesLength,
                     backgroundMapData, backgroundMapLength));
  bg->useMapScreenBlock(24);
  bg->useCharBlock(0);

  // TextStream::instance().setText(std::to_string(library->getCount()), 5, 0);
  // u32 row = 6;
  // for (auto& songFile : library->getSongs(0, 0)) {
  //   TextStream::instance().setText(songFile->getAudioFile(), row, 0);
  //   row++;
  // }
}

void SelectionScene::tick(u16 keys) {
  if (i == 0) {
    BACKGROUND_enable(true, true, false, false);
    i = 1;

    REG_BG0CNT = BG_CBB(3) | BG_SBB(25) | BG_8BPP | BG_REG_32x32;

    // Set up palette memory, colors are 15bpp
    // pal_bg_mem[0] = 0;      // base color (black)
    pal_bg_mem[255] = 127;  // red

    // Set up 8x8 tiles from 224 to 255
    for (int tile = 224; tile <= 255; tile++) {
      // RED TILE
      for (int line = 0; line < 8; line++) {
        // update charblock 3, tile tile, line i * 2

        tile8_mem[3][tile].data[line * 2] =
            (255 << 0) + (255 << 8) + (255 << 16) + (255 << 24);
        tile8_mem[3][tile].data[line * 2 + 1] =
            (255 << 0) + (255 << 8) + (255 << 16) + (255 << 24);
      }
    }

    // Set up an 8x8 tile 254
    // TRANSPARENT TILE
    for (int line = 0; line < 8; line++) {
      // update charblock 3, tile 254, line i * 2

      tile8_mem[3][254].data[line * 2] =
          (0 << 0) + (0 << 8) + (0 << 16) + (0 << 24);
      tile8_mem[3][254].data[line * 2 + 1] =
          (0 << 0) + (0 << 8) + (0 << 16) + (0 << 24);
    }

    // Set up a map, draw tiles
    for (int i = 0; i < 32 * 32; i++)
      // update screenblock 25, screenblock entry i
      // set tile 255 or 254 (transparent)
      se_mem[25][i] = i < 7 ? 254 : 255;

    REG_BLDCNT = 0b0000001001000001;  // blend BG0 on top of BG1
    REG_BLDALPHA = 0b0000100000011000;
    // BG0 weight: 11000, BG1 weight: 1000
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
      name = (char*)"Run to You";
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

SelectionScene::~SelectionScene() {}
