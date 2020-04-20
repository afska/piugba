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
    4244438016, 4244438268, 4244438016, 4244438268, 4244438016, 4244438268,
    4244438016, 4244438268, 4244438016, 4244438268, 4244438016, 4244438268,
    4244438016, 4244438268, 0,          0,          4244438268, 4244438268,
    4244438268, 4244438268, 4244438268, 4244438268, 4244438268, 4244438268,
    4244438268, 4244438268, 4244438268, 4244438268, 4244438268, 4244438268,
    0,          0,          16579836,   4261281024, 16579836,   4261281024,
    16579836,   4261281024, 16579836,   4261281024, 16579836,   4261281024,
    16579836,   4261281024, 16579836,   4261281024, 0,          0,
    4261281277, 4261281277, 4261281277, 4261281277, 4261281277, 4261281277,
    4261281277, 4261281277, 4261281277, 4261281277, 4261281277, 4261281277,
    4261281277, 4261281277, 0,          0,          4261281277, 16645629,
    4261281277, 16645629,   4261281277, 16645629,   4261281277, 16645629,
    4261281277, 16645629,   4261281277, 16645629,   4261281277, 16645629,
    0,          0,          4278124032, 4278124286, 4278124032, 4278124286,
    4278124032, 4278124286, 4278124032, 4278124286, 4278124032, 4278124286,
    4278124032, 4278124286, 4278124032, 4278124286, 0,          0,
    4278124286, 4278124286, 4278124286, 4278124286, 4278124286, 4278124286,
    4278124286, 4278124286, 4278124286, 4278124286, 4278124286, 4278124286,
    4278124286, 4278124286, 0,          0,          16711422,   4294967040,
    16711422,   4294967040, 16711422,   4294967040, 16711422,   4294967040,
    16711422,   4294967040, 16711422,   4294967040, 16711422,   4294967040,
    0,          0,          4294967295, 4294967295, 4294967295, 4294967295,
    4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295,
    4294967295, 4294967295, 4294967295, 4294967295, 0,          0,
    4294967295, 16777215,   4294967295, 16777215,   4294967295, 16777215,
    4294967295, 16777215,   4294967295, 16777215,   4294967295, 16777215,
    4294967295, 16777215,   4244438016, 4244438268, 4244438016, 4244438268,
    4244438016, 4244438268, 4244438016, 4244438268, 4244438016, 4244438268,
    4244438016, 4244438268, 4244438016, 4244438268, 4244438016, 4244438268,
    4244438268, 4244438268, 4244438268, 4244438268, 4244438268, 4244438268,
    4244438268, 4244438268, 4244438268, 4244438268, 4244438268, 4244438268,
    4244438268, 4244438268, 4244438268, 4244438268, 16579836,   4261281024,
    16579836,   4261281024, 16579836,   4261281024, 16579836,   4261281024,
    16579836,   4261281024, 16579836,   4261281024, 16579836,   4261281024,
    16579836,   4261281024, 4261281277, 4261281277, 4261281277, 4261281277,
    4261281277, 4261281277, 4261281277, 4261281277, 4261281277, 4261281277,
    4261281277, 4261281277, 4261281277, 4261281277, 4261281277, 4261281277,
    4261281277, 16645629,   4261281277, 16645629,   4261281277, 16645629,
    4261281277, 16645629,   4261281277, 16645629,   4261281277, 16645629,
    4261281277, 16645629,   4261281277, 16645629,   4278124032, 4278124286,
    4278124032, 4278124286, 4278124032, 4278124286, 4278124032, 4278124286,
    4278124032, 4278124286, 4278124032, 4278124286, 4278124032, 4278124286,
    4278124032, 4278124286, 4278124286, 4278124286, 4278124286, 4278124286,
    4278124286, 4278124286, 4278124286, 4278124286, 4278124286, 4278124286,
    4278124286, 4278124286, 4278124286, 4278124286, 4278124286, 4278124286,
    16711422,   4294967040, 16711422,   4294967040, 16711422,   4294967040,
    16711422,   4294967040, 16711422,   4294967040, 16711422,   4294967040,
    16711422,   4294967040, 16711422,   4294967040, 4294967295, 4294967295,
    4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295,
    4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295,
    4294967295, 4294967295, 4294967295, 16777215,   4294967295, 16777215,
    4294967295, 16777215,   4294967295, 16777215,   4294967295, 16777215,
    4294967295, 16777215,   4294967295, 16777215,   4294967295, 16777215,
    4244438016, 4244438268, 4244438016, 4244438268, 4244438016, 4244438268,
    4244438016, 4244438268, 4244438016, 4244438268, 0,          0,
    0,          0,          0,          0,          4244438268, 4244438268,
    4244438268, 4244438268, 4244438268, 4244438268, 4244438268, 4244438268,
    4244438268, 4244438268, 0,          0,          0,          0,
    0,          0,          16579836,   4261281024, 16579836,   4261281024,
    16579836,   4261281024, 16579836,   4261281024, 16579836,   4261281024,
    0,          0,          0,          0,          0,          0,
    4261281277, 4261281277, 4261281277, 4261281277, 4261281277, 4261281277,
    4261281277, 4261281277, 4261281277, 4261281277, 0,          0,
    0,          0,          0,          0,          4261281277, 16645629,
    4261281277, 16645629,   4261281277, 16645629,   4261281277, 16645629,
    4261281277, 16645629,   0,          0,          0,          0,
    0,          0,          4278124032, 4278124286, 4278124032, 4278124286,
    4278124032, 4278124286, 4278124032, 4278124286, 4278124032, 4278124286,
    0,          0,          0,          0,          0,          0,
    4278124286, 4278124286, 4278124286, 4278124286, 4278124286, 4278124286,
    4278124286, 4278124286, 4278124286, 4278124286, 0,          0,
    0,          0,          0,          0,          16711422,   4294967040,
    16711422,   4294967040, 16711422,   4294967040, 16711422,   4294967040,
    16711422,   4294967040, 0,          0,          0,          0,
    0,          0,          4294967295, 4294967295, 4294967295, 4294967295,
    4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295,
    0,          0,          0,          0,          0,          0,
    4294967295, 16777215,   4294967295, 16777215,   4294967295, 16777215,
    4294967295, 16777215,   4294967295, 16777215,   0,          0,
    0,          0,          0,          0};
const u32 MASK_MAP[] = {
    225, 226, 226, 226, 226, 226, 226, 227, 228, 228, 228, 228, 228, 228, 229,
    230, 231, 231, 231, 231, 231, 231, 232, 233, 233, 233, 233, 233, 233, 234,
    224, 224, 235, 236, 236, 236, 236, 236, 236, 237, 238, 238, 238, 238, 238,
    238, 239, 240, 241, 241, 241, 241, 241, 241, 242, 243, 243, 243, 243, 243,
    243, 244, 224, 224, 235, 236, 236, 236, 236, 236, 236, 237, 238, 238, 238,
    238, 238, 238, 239, 240, 241, 241, 241, 241, 241, 241, 242, 243, 243, 243,
    243, 243, 243, 244, 224, 224, 235, 236, 236, 236, 236, 236, 236, 237, 238,
    238, 238, 238, 238, 238, 239, 240, 241, 241, 241, 241, 241, 241, 242, 243,
    243, 243, 243, 243, 243, 244, 224, 224, 235, 236, 236, 236, 236, 236, 236,
    237, 238, 238, 238, 238, 238, 238, 239, 240, 241, 241, 241, 241, 241, 241,
    242, 243, 243, 243, 243, 243, 243, 244, 224, 224, 245, 246, 246, 246, 246,
    246, 246, 247, 248, 248, 248, 248, 248, 248, 249, 250, 251, 251, 251, 251,
    251, 251, 252, 253, 253, 253, 253, 253, 253, 254, 224, 224};
const u32 MASK_PALETTE[] = {31, 992, 31744, 32767};
const u32 TILES_BANK = 2;
const u32 TILES_START_INDEX = 224;
const u32 TILES_END_INDEX = 256;
const u32 TILES_LENGTH = 496;
const u32 TILE_SIZE = 16;
const u32 MAP_BANK = 18;
const u32 MAP_START_INDEX = 224;
const u32 MAP_END_INDEX = 416;
const u32 MAP_TOTAL_TILES = 1024;
const u32 PALETTE_START_INDEX = 252;
const u32 PALETTE_END_INDEX = 256;
const u32 PALETTE_LENGTH = 4;

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
  bg = std::unique_ptr<Background>(new Background(
      ID_MAIN_BACKGROUND, backgroundTilesData, backgroundTilesLength,
      backgroundMapData, backgroundMapLength));
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);

  TextStream::instance().setText("Run to You", 15, 6);
}

void SelectionScene::tick(u16 keys) {
  if (i == 0) {
    BACKGROUND_enable(true, true, true, false);
    i = 1;

    REG_BGCNT[ID_HIGHLIGHTER] = BG_CBB(TILES_BANK) | BG_SBB(MAP_BANK) |
                                BG_8BPP | BG_REG_32x32 | ID_HIGHLIGHTER;

    // palette (palette memory)
    for (u32 colorIndex = 0; colorIndex < PALETTE_LENGTH; colorIndex++)
      pal_bg_mem[PALETTE_START_INDEX + colorIndex] = MASK_PALETTE[colorIndex];

    // TODO: figure out how to make transparent pixels
    // ^^^^ generar 4 tilemaps desde el optimizer y cambiarlos?
    pal_bg_mem[PALETTE_START_INDEX] = 0x0031;
    pal_bg_mem[PALETTE_START_INDEX + 1] = 0;
    pal_bg_mem[PALETTE_START_INDEX + 2] = 0;
    // pal_bg_mem[PALETTE_START_INDEX + 3] = 0;
    // TODO: use 1 color less to leave 255 reserved for font color

    // tiles (charblocks)
    u32 tileIndex = 0;
    int part = -1;
    for (u32 i = 0; i < TILES_LENGTH; i++) {
      part++;
      if (part == TILE_SIZE) {
        tileIndex++;
        part = 0;
      }

      tile8_mem[TILES_BANK][TILES_START_INDEX + tileIndex].data[part] =
          MASK_TILES[i];
    }

    // map (screenblocks)
    for (u32 mapIndex = 0; mapIndex < MAP_TOTAL_TILES; mapIndex++)
      se_mem[MAP_BANK][mapIndex] =
          mapIndex >= MAP_START_INDEX && mapIndex < MAP_END_INDEX
              ? MASK_MAP[mapIndex - MAP_START_INDEX]
              : TILES_START_INDEX;  // (transparent)

    // blend BG1 on top of BG2
    // BG1 weight: 11000, BG2 weight: 1000
    REG_BLDCNT = 0b0000010001000010;
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
