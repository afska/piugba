#include "SelectionScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "gameplay/models/Song.h"
#include "scenes/SongScene.h"
#include "utils/SpriteUtils.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

static Library library;

SelectionScene::SelectionScene(std::shared_ptr<GBAEngine> engine)
    : Scene(engine) {}

std::vector<Background*> SelectionScene::backgrounds() {
  return {/*bg.get()*/};
}

std::vector<Sprite*> SelectionScene::sprites() {
  std::vector<Sprite*> sprites;

  return sprites;
}

void SelectionScene::load() {
  // BACKGROUND_enable(true, true, false, false);
  // engine->disableText();

  // const GBFS_FILE* fs = find_first_gbfs_file(0);
  // u32 backgroundPaletteLength;
  // auto backgroundPaletteData =
  //     (COLOR*)gbfs_get_obj(fs, "output.pal.bin", &backgroundPaletteLength);
  // backgroundPalette =
  //     std::unique_ptr<BackgroundPaletteManager>(new BackgroundPaletteManager(
  //         backgroundPaletteData, backgroundPaletteLength));

  // u32 backgroundTilesLength, backgroundMapLength;
  // auto backgroundTilesData =
  //     gbfs_get_obj(fs, "output.img.bin", &backgroundTilesLength);
  // auto backgroundMapData =
  //     gbfs_get_obj(fs, "output.map.bin", &backgroundMapLength);
  // bg = std::unique_ptr<Background>(
  //     new Background(1, backgroundTilesData, backgroundTilesLength,
  //                    backgroundMapData, backgroundMapLength));
  // bg->useMapScreenBlock(24);
  // bg->useCharBlock(0);

  u32 row = 6;
  for (auto& songFile : library.getSongs()) {
    TextStream::instance().setText(songFile->getAudioFile(), row, 0);
    row++;
  }

  // TextStream::instance().setText("SEL - Don't Bother Me (HARD)", 6, 0);
  // TextStream::instance().setText("ARR - Witch Doctor (CRAZY)", 8, 0);
  // TextStream::instance().setText("B - Beat of the War 2 (16)", 9, 0);
  // TextStream::instance().setText("A - Tepris (CRAZY)", 10, 0);
  // TextStream::instance().setText("L - Beethoven Virus (HARD)", 12, 0);
  // TextStream::instance().setText("R - Beethoven Virus (CRAZY)", 13, 0);
  // TextStream::instance().setText("IZQ - Run to You (HARD)", 15, 0);
  // TextStream::instance().setText("DER - Run to You (CRAZY)", 16, 0);
  // TextStream::instance().setText("START - Extravaganza (CRAZY)", 18, 0);
}

void SelectionScene::tick(u16 keys) {
  // if (i == 0) {
  //   i = 1;

  //   REG_BG0CNT = BG_CBB(3) | BG_SBB(30) | BG_8BPP | BG_REG_32x32;

  //   // Set up palette memory, colors are 15bpp
  //   // pal_bg_mem[0] = 0;      // base color (black)
  //   pal_bg_mem[255] = 127;  // red

  //   // Set up an 8x8 tile 254
  //   // TRANSPARENT TILE
  //   for (int line = 0; line < 8; line++) {
  //     // update charblock 3, tile 254, line i * 2

  //     tile8_mem[3][254].data[line * 2] =
  //         (0 << 0) + (0 << 8) + (0 << 16) + (0 << 24);
  //     tile8_mem[3][254].data[line * 2 + 1] =
  //         (0 << 0) + (0 << 8) + (0 << 16) + (0 << 24);
  //   }

  //   // Set up an 8x8 tile 255
  //   // RED TILE
  //   for (int line = 0; line < 8; line++) {
  //     // update charblock 3, tile 255, line i * 2

  //     tile8_mem[3][255].data[line * 2] =
  //         (255 << 0) + (255 << 8) + (255 << 16) + (255 << 24);
  //     tile8_mem[3][255].data[line * 2 + 1] =
  //         (255 << 0) + (255 << 8) + (255 << 16) + (255 << 24);
  //   }

  //   // Set up a map, draw tiles
  //   for (int i = 0; i < 32 * 32; i++)
  //     // update screenblock 30, screenblock entry i
  //     se_mem[30][i] = i < 7 ? 254 : 255;  // set tile 255 or 254
  //     (transparent)

  //   REG_BLDCNT = 0b0000001001000001;    // blend BG0 on top of BG1
  //   REG_BLDALPHA = 0b0000100000011000;  // BG0 weight: 11000, BG1 weight:
  //   1000
  // }

  if (keys & KEY_ANY && !engine->isTransitioning()) {
    const GBFS_FILE* fs = find_first_gbfs_file(0);

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