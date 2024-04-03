const utils = require("../utils");
const $path = require("path");

const COMMAND = (input, output) =>
  `ffmpeg -y -i "${input}" -ac 1 -ar 36314 -f s8 "${output}"`;
const EXTENSION = "aud.bin";

module.exports = async (name, filePath, outputPath) => {
  await utils.run(
    COMMAND(filePath, $path.join(outputPath, `${name}.${EXTENSION}`))
  );
};
