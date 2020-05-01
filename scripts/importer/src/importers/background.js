const utils = require("../utils");
const $path = require("path");
const _ = require("lodash");

const COMMAND_1 = (input, output) =>
  `magick "${input}" -resize 240x160\! -colors 254 "${output}"`;
const COMMAND_2 = (input) => `grit "${input}" -gt -gB8 -mRtf -mLs -ftb`;
const COMMAND_3 = (tmp1, tmp2) => `rm "${tmp1}" && rm "${tmp2}"`;
const COMMAND_FIX = (input) => `pngfix -f "${input}`;
const EXTENSION_TMP1 = "bmp";
const EXTENSION_TMP2 = "h";
const EXTENSION_FIXED = "-fixed.png";

module.exports = (name, filePath, outputPath) => {
  const tempFile1 = $path.join(outputPath, `${name}.${EXTENSION_TMP1}`);
  const tempFile2 = $path.join(outputPath, `${name}.${EXTENSION_TMP2}`);
  try {
    utils.run(COMMAND_1(filePath, tempFile1));
  } catch (e) {
    console.log("  ⚠️  fixing background...");
    utils.run(COMMAND_FIX(filePath));
    const fixedFilePath =
      (_.endsWith(filePath, ".png") ? filePath.slice(0, -4) : filePath) +
      EXTENSION_FIXED;
    utils.run(COMMAND_1(fixedFilePath, tempFile1));
  }
  utils.run(COMMAND_2(tempFile1), { cwd: outputPath });
  utils.run(COMMAND_3(tempFile1, tempFile2));
};
