const utils = require("../utils");
const $path = require("path");

const COMMAND = (input, output) =>
  `ffmpeg -y -i "${input}" -ac 1 -af aresample=18157 -strict unofficial -c:a gsm "${output}"`;
const EXTENSION = "gsm";

module.exports = async (name, filePath, outputPath) => {
  await utils.run(
    COMMAND(filePath, $path.join(outputPath, `${name}.${EXTENSION}`))
  );
};
