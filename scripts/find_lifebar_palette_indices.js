const fs = require("fs");

const FILE_PATH = "../src/data/content/compiled/palette_song.c";
const LIFEBAR_COLORS = [
  0x007F, 0x10F9,
  0x019A, 0x1DB6,
  0x0A7E, 0x063B,
  0x02FE, 0x02BC,
  0x039F, 0x037E,
  0x03DC, 0x039B,
  0x03F9, 0x03B7,
  0x03CE, 0x036F,
  0x23EF, 0x03A8
];

const file = fs.readFileSync(FILE_PATH).toString();
const content = file.substring(file.indexOf("0x"), file.indexOf("};"));
const palette = eval(`[${content}]`);

const indices = LIFEBAR_COLORS.map((color) => palette.indexOf(color));

console.log(JSON.stringify({ colors: LIFEBAR_COLORS, indices }));
