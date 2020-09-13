const utils = require("../utils");
const $path = require("path");
const _ = require("lodash");

const COMMAND_BUILD = (input, output) =>
  `magick "${input}" -resize 240x160\! -colors 254 "${output}"`;
const COMMAND_BUILD_REMAP = (input, firstColorPalette, output) =>
  `magick "${input}" -resize 240x160\! -colors 253 -unique-colors ${TMP_PALETTE} && ` +
  `magick "${firstColorPalette}" _tmp_palette.bmp +append ${TMP_PALETTE} && ` +
  `magick "${input}" -resize 240x160\! -colors 253 -remap ${TMP_PALETTE} "${output}" && ` +
  `rm ${TMP_PALETTE}`;
const COMMAND_ENCODE = (input) => `grit "${input}" -gt -gB8 -mRtf -mLs -ftb`;
const COMMAND_CLEANUP = (tmp1, tmp2) => `rm "${tmp1}" && rm "${tmp2}"`;
const COMMAND_FIX = (input) => `pngfix -f "${input}`;
const TMP_PALETTE = "_tmp_palette.bmp";
const EXTENSION_TMP1 = "bmp";
const EXTENSION_TMP2 = "h";
const EXTENSION_FIXED = "-fixed.png";

const FIX_FILE_PATH = (path) =>
  (_.endsWith(path, ".png") ? path.slice(0, -4) : path) + EXTENSION_FIXED;
//                  ^ not a mistake, it needs to be case-sensitive because of pngfix

module.exports = (name, filePath, outputPath, transparentColor = null) => {
  const tempFile1 = $path.join(outputPath, `${name}.${EXTENSION_TMP1}`);
  const tempFile2 = $path.join(outputPath, `${name}.${EXTENSION_TMP2}`);

  try {
    utils.run(
      transparentColor !== null
        ? COMMAND_BUILD_REMAP(filePath, transparentColor, tempFile1)
        : COMMAND_BUILD(filePath, tempFile1)
    );
  } catch (e) {
    console.log("  ⚠️  fixing background...");
    utils.run(COMMAND_FIX(filePath));
    const fixedFilePath = FIX_FILE_PATH(filePath);
    utils.run(COMMAND_BUILD(fixedFilePath, tempFile1));
  }

  utils.run(COMMAND_ENCODE(tempFile1), { cwd: outputPath });
  utils.run(COMMAND_CLEANUP(tempFile1, tempFile2));
};

module.exports.FIX_FILE_PATH = FIX_FILE_PATH;
