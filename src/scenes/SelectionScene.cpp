#include "SelectionScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "gameplay/models/Song.h"
#include "scenes/SongScene.h"
#include "utils/SpriteUtils.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

const u32 MASK_TILES[] = {
    0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,
    4261281024, 4261281277, 4261281024, 4261281277, 4261281024, 4261281277,
    4261281024, 4261281277, 4261281024, 4261281277, 4261281024, 4261281277,
    4261281024, 4261281277, 0,          0,          4261281277, 4261281277,
    4261281277, 4261281277, 4261281277, 4261281277, 4261281277, 4261281277,
    4261281277, 4261281277, 4261281277, 4261281277, 4261281277, 4261281277,
    0,          0,          16645629,   4294967040, 16645629,   4294967040,
    16645629,   4294967040, 16645629,   4294967040, 16645629,   4294967040,
    16645629,   4294967040, 16645629,   4294967040, 0,          0,
    4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295,
    4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295,
    4294967295, 4294967295, 0,          0,          4294967295, 16777215,
    4294967295, 16777215,   4294967295, 16777215,   4294967295, 16777215,
    4294967295, 16777215,   4294967295, 16777215,   4294967295, 16777215,
    0,          0,          4244438016, 4244438268, 4244438016, 4244438268,
    4244438016, 4244438268, 4244438016, 4244438268, 4244438016, 4244438268,
    4244438016, 4244438268, 4244438016, 4244438268, 0,          0,
    4244438268, 4244438268, 4244438268, 4244438268, 4244438268, 4244438268,
    4244438268, 4244438268, 4244438268, 4244438268, 4244438268, 4244438268,
    4244438268, 4244438268, 0,          0,          16579836,   4278124032,
    16579836,   4278124032, 16579836,   4278124032, 16579836,   4278124032,
    16579836,   4278124032, 16579836,   4278124032, 16579836,   4278124032,
    0,          0,          4278124286, 4278124286, 4278124286, 4278124286,
    4278124286, 4278124286, 4278124286, 4278124286, 4278124286, 4278124286,
    4278124286, 4278124286, 4278124286, 4278124286, 0,          0,
    4278124286, 16711422,   4278124286, 16711422,   4278124286, 16711422,
    4278124286, 16711422,   4278124286, 16711422,   4278124286, 16711422,
    4278124286, 16711422,   4261281024, 4261281277, 4261281024, 4261281277,
    4261281024, 4261281277, 4261281024, 4261281277, 4261281024, 4261281277,
    4261281024, 4261281277, 4261281024, 4261281277, 4261281024, 4261281277,
    4261281277, 4261281277, 4261281277, 4261281277, 4261281277, 4261281277,
    4261281277, 4261281277, 4261281277, 4261281277, 4261281277, 4261281277,
    4261281277, 4261281277, 4261281277, 4261281277, 16645629,   4294967040,
    16645629,   4294967040, 16645629,   4294967040, 16645629,   4294967040,
    16645629,   4294967040, 16645629,   4294967040, 16645629,   4294967040,
    16645629,   4294967040, 4294967295, 4294967295, 4294967295, 4294967295,
    4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295,
    4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295,
    4294967295, 16777215,   4294967295, 16777215,   4294967295, 16777215,
    4294967295, 16777215,   4294967295, 16777215,   4294967295, 16777215,
    4294967295, 16777215,   4294967295, 16777215,   4244438016, 4244438268,
    4244438016, 4244438268, 4244438016, 4244438268, 4244438016, 4244438268,
    4244438016, 4244438268, 4244438016, 4244438268, 4244438016, 4244438268,
    4244438016, 4244438268, 4244438268, 4244438268, 4244438268, 4244438268,
    4244438268, 4244438268, 4244438268, 4244438268, 4244438268, 4244438268,
    4244438268, 4244438268, 4244438268, 4244438268, 4244438268, 4244438268,
    16579836,   4278124032, 16579836,   4278124032, 16579836,   4278124032,
    16579836,   4278124032, 16579836,   4278124032, 16579836,   4278124032,
    16579836,   4278124032, 16579836,   4278124032, 4278124286, 4278124286,
    4278124286, 4278124286, 4278124286, 4278124286, 4278124286, 4278124286,
    4278124286, 4278124286, 4278124286, 4278124286, 4278124286, 4278124286,
    4278124286, 4278124286, 4278124286, 16711422,   4278124286, 16711422,
    4278124286, 16711422,   4278124286, 16711422,   4278124286, 16711422,
    4278124286, 16711422,   4278124286, 16711422,   4278124286, 16711422,
    4261281024, 4261281277, 4261281024, 4261281277, 4261281024, 4261281277,
    4261281024, 4261281277, 4261281024, 4261281277, 0,          0,
    0,          0,          0,          0,          4261281277, 4261281277,
    4261281277, 4261281277, 4261281277, 4261281277, 4261281277, 4261281277,
    4261281277, 4261281277, 0,          0,          0,          0,
    0,          0,          16645629,   4294967040, 16645629,   4294967040,
    16645629,   4294967040, 16645629,   4294967040, 16645629,   4294967040,
    0,          0,          0,          0,          0,          0,
    4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295,
    4294967295, 4294967295, 4294967295, 4294967295, 0,          0,
    0,          0,          0,          0,          4294967295, 16777215,
    4294967295, 16777215,   4294967295, 16777215,   4294967295, 16777215,
    4294967295, 16777215,   0,          0,          0,          0,
    0,          0,          4244438016, 4244438268, 4244438016, 4244438268,
    4244438016, 4244438268, 4244438016, 4244438268, 4244438016, 4244438268,
    0,          0,          0,          0,          0,          0,
    4244438268, 4244438268, 4244438268, 4244438268, 4244438268, 4244438268,
    4244438268, 4244438268, 4244438268, 4244438268, 0,          0,
    0,          0,          0,          0,          16579836,   4278124032,
    16579836,   4278124032, 16579836,   4278124032, 16579836,   4278124032,
    16579836,   4278124032, 0,          0,          0,          0,
    0,          0,          4278124286, 4278124286, 4278124286, 4278124286,
    4278124286, 4278124286, 4278124286, 4278124286, 4278124286, 4278124286,
    0,          0,          0,          0,          0,          0,
    4278124286, 16711422,   4278124286, 16711422,   4278124286, 16711422,
    4278124286, 16711422,   4278124286, 16711422,   0,          0,
    0,          0,          0,          0};
const u32 MASK_MAP[] = {
    226, 227, 449, 449, 449, 449, 449, 228, 229, 456, 456, 456, 456, 456, 230,
    231, 232, 464, 464, 464, 464, 464, 233, 234, 471, 471, 471, 471, 471, 235,
    224, 224, 236, 237, 481, 481, 481, 481, 481, 238, 239, 488, 488, 488, 488,
    488, 240, 241, 242, 496, 496, 496, 496, 496, 243, 244, 503, 503, 503, 503,
    503, 245, 224, 224, 480, 481, 481, 481, 481, 481, 481, 487, 488, 488, 488,
    488, 488, 488, 494, 495, 496, 496, 496, 496, 496, 496, 502, 503, 503, 503,
    503, 503, 503, 509, 224, 224, 480, 481, 481, 481, 481, 481, 481, 487, 488,
    488, 488, 488, 488, 488, 494, 495, 496, 496, 496, 496, 496, 496, 502, 503,
    503, 503, 503, 503, 503, 509, 224, 224, 480, 481, 481, 481, 481, 481, 481,
    487, 488, 488, 488, 488, 488, 488, 494, 495, 496, 496, 496, 496, 496, 496,
    502, 503, 503, 503, 503, 503, 503, 509, 224, 224, 246, 247, 609, 609, 609,
    609, 609, 248, 249, 616, 616, 616, 616, 616, 250, 251, 252, 624, 624, 624,
    624, 624, 253, 254, 631, 631, 631, 631, 631, 255, 224, 224};
const u32 MASK_PALETTE[] = {31744, 31, 32767, 992};
const u32 TILE_BANK = 3;
const u32 TILE_SIZE = 16;
const u32 TILE_START_INDEX = 224;
const u32 TILE_END_INDEX = 256;
const u32 MAP_BANK = 25;
const u32 MAP_START_INDEX = 224;
const u32 MAP_END_INDEX = 416;
const u32 MAP_TOTAL_TILES = 1024;
const u32 PALETTE_START_INDEX = 252;
const u32 PALETTE_END_INDEX = 256;

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

    REG_BG0CNT = BG_CBB(TILE_BANK) | BG_SBB(MAP_BANK) | BG_8BPP | BG_REG_32x32;

    // palette (palette memory)
    for (u32 colorIndex = 0; colorIndex < sizeof(MASK_PALETTE); colorIndex++)
      pal_bg_mem[PALETTE_START_INDEX + colorIndex] = MASK_PALETTE[colorIndex];

    // tiles (charblocks)
    u32 tileIndex = 0;
    u32 part = 0;
    for (u32 i = 0; i < sizeof(MASK_TILES); i++) {
      tile8_mem[TILE_BANK][TILE_START_INDEX + tileIndex].data[part] =
          MASK_TILES[i];

      if (part == TILE_SIZE) {
        tileIndex++;
        part = 0;
      } else
        part++;
    }

    // map (screenblocks)
    for (u32 mapIndex = 0; mapIndex < MAP_TOTAL_TILES; mapIndex++)
      se_mem[MAP_BANK][i] =
          mapIndex >= MAP_START_INDEX && mapIndex < MAP_END_INDEX
              ? MASK_MAP[i]
              : TILE_START_INDEX;  // (transparent)

    // blend BG0 on top of BG1
    // BG0 weight: 11000, BG1 weight: 1000
    REG_BLDCNT = 0b0000001001000001;
    REG_BLDALPHA = 0b0000100000011000;
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
