const fs = require("fs");
const _ = require("./importer/node_modules/lodash");

const FILE_PATH = "../src/data/content/_compiled_sprites/palette_song.c";
const LIFEBAR_COLORS = {
  player0: [
    0x007F, 0x10F9,
    0x019A, 0x1DB6,
    0x0A7E, 0x063B,
    0x02FE, 0x02BC,
    0x039F, 0x037E,
    0x03DC, 0x039B,
    0x03F9, 0x03B7,
    0x03CE, 0x036F,
    0x23EF, 0x03A8
  ],
  player1: [
    0x007E, 0x10F8,
    0x0199, 0x1DB5,
    0x065D, 0x061A,
    0x02DD, 0x029B,
    0x037E, 0x033D,
    0x039B, 0x037A,
    0x03D8, 0x0396,
    0x03AE, 0x034E,
    0x1FCE, 0x0388
  ]
};

const file = fs.readFileSync(FILE_PATH).toString();
const content = file.substring(file.indexOf("0x"), file.indexOf("};"));
const palette = eval(`[${content}]`);

console.log(JSON.stringify(
  _.mapValues(LIFEBAR_COLORS, (colors) => ({
    colors,
    indices: colors.map((color) => palette.indexOf(color))
  }))
));
