const fs = require("fs");
const _ = require("./importer/node_modules/lodash");

const FILE_PATH = "../src/data/content/_compiled_sprites/palette_song.c";
const LIFEBAR_COLORS = {
  // TODO: CLASSIC LIFEBAR
  player0: [
    0x3f, 0x15e, 0x21e, 0x67e, 0x2de, 0x35d, 0x39f, 0x3dd, 0x37a, 0x396, 0x352,
    0x329, 0x3c0, 0x23c0, 0x4bc0, 0x5340, 0x6362, 0x6b40, 0x66a0, 0x6660,
    0x7e80, 0x7a20, 0x7560, 0x78c0, 0x7804, 0x6c0d, 0x6814,
  ],
  player1: [
    0x3d, 0x15c, 0x1fc, 0x25c, 0x2bc, 0x31b, 0x31d, 0x33c, 0x338, 0x355, 0x311,
    0x2e8, 0x380, 0x2380, 0x4780, 0x5760, 0x5b22, 0x6700, 0x5e60, 0x5e20,
    0x7660, 0x7200, 0x6d60, 0x70c0, 0x7004, 0x640c, 0x6013,
  ],
};

const file = fs.readFileSync(FILE_PATH).toString();
const content = file.substring(file.indexOf("0x"), file.indexOf("};"));
const palette = eval(`[${content}]`);

console.log(
  JSON.stringify(
    _.mapValues(LIFEBAR_COLORS, (colors, player) => {
      return {
        colors,
        indexes: colors.map((color, i) => {
          const index = palette.indexOf(color);
          if (index === -1) {
            console.error(`Cannot find color #${i} of ${player}!`);
            process.exit(1);
          }
          return index;
        }),
      };
    })
  )
);
