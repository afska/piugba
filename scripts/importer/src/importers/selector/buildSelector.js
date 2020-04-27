const _ = require("lodash");

module.exports = (count) => {
  return (
    `<?xml version="1.0" encoding="UTF-8"?>\n` +
    `<group>\n` +
    _.range(1, count + 1).map((n) => {
      return (
        `<image id="song${n}">\n` +
        `<read filename="song${n}.png" />\n` +
        `<resize geometry="58x44!" />\n` +
        `</image>\n`
      );
    }) +
    `<image>\n` +
    `<read filename="selector.bmp" />\n` +
    (count >= 1 ? `<composite image="song1" geometry="+1+57" />\n` : "") +
    (count >= 2 ? `<composite image="song2" geometry="+61+57" />\n` : "") +
    (count >= 3 ? `<composite image="song3" geometry="+121+57" />\n` : "") +
    (count >= 4 ? `<composite image="song4" geometry="+181+57" />\n` : "") +
    `<quantize colors="250" />\n` +
    `</image>\n` +
    `<write filename="output.bmp" />\n` +
    `</group>\n`
  );
};
