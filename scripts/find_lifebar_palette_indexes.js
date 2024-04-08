const fs = require("fs");
const _ = require("./importer/node_modules/lodash");

const FILE_PATH = "../src/data/content/_compiled_sprites/palette_song.c";
const LIFEBAR_COLORS = {
  player0: [
    0x007f, 0x10f9, 0x019a, 0x1db6, 0x0a7e, 0x063b, 0x02fe, 0x02bc, 0x039f,
    0x037e, 0x03dc, 0x039b, 0x03f9, 0x03b7, 0x03ce, 0x036f, 0x23ef, 0x03a8,
  ],
  player1: [
    0x007e, 0x10f8, 0x0199, 0x1db5, 0x065d, 0x061a, 0x02dd, 0x029b, 0x035e,
    0x033d, 0x039a, 0x037a, 0x03d8, 0x0396, 0x03ae, 0x034e, 0x1fce, 0x0388,
  ],
};

const file = fs.readFileSync(FILE_PATH).toString();
const content = file.substring(file.indexOf("0x"), file.indexOf("};"));
const palette = eval(`[${content}]`);

console.log(
  JSON.stringify(
    _.mapValues(LIFEBAR_COLORS, (colors) => ({
      colors,
      indexes: colors.map((color) => palette.indexOf(color)),
    }))
  )
);
