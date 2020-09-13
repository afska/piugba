const utils = require("../utils");
const $path = require("path");
const _ = require("lodash");

const COMMAND_BUILD = (input, output) =>
  `magick "${input}" -resize ${RESOLUTION} -colors 254 "${output}"`;
const COMMAND_BUILD_REMAP = (input, firstColorPalette, tempPalette, output) =>
  `magick "${input}" -resize ${RESOLUTION} -colors ${COLORS} -unique-colors "${tempPalette}" && ` +
  `magick "${firstColorPalette}" "${tempPalette}" +append "${tempPalette}" && ` +
  `magick "${input}" -resize ${RESOLUTION} -colors ${COLORS} -remap "${tempPalette}" "${output}" && ` +
  `rm "${tempPalette}"`;
const COMMAND_ENCODE = (input) => `grit "${input}" -gt -gB8 -mRtf -mLs -ftb`;
const COMMAND_CLEANUP = (tmp1, tmp2) => `rm "${tmp1}" && rm "${tmp2}"`;
const COMMAND_FIX = (input) => `pngfix -f "${input}`;
const RESOLUTION = "240x160!";
const COLORS = "253";
const EXTENSIONS_TMP = ["pal.bpm", "bmp", "h"];
const EXTENSION_FIXED = "-fixed.png";

const FIX_FILE_PATH = (path) =>
  (_.endsWith(path, ".png") ? path.slice(0, -4) : path) + EXTENSION_FIXED;
//                  ^ not a mistake, it needs to be case-sensitive because of pngfix

module.exports = (name, filePath, outputPath, transparentColor = null) => {
  const tempFiles = EXTENSIONS_TMP.map((it) =>
    $path.join(outputPath, `${name}.${it}`)
  );

  try {
    utils.run(
      transparentColor !== null
        ? COMMAND_BUILD_REMAP(
            filePath,
            transparentColor,
            tempFiles[0],
            tempFiles[1]
          )
        : COMMAND_BUILD(filePath, tempFiles[1])
    );
  } catch (originalException) {
    try {
      console.log("  ⚠️  fixing background...");
      utils.run(COMMAND_FIX(filePath));
      const fixedFilePath = FIX_FILE_PATH(filePath);
      utils.run(COMMAND_BUILD(fixedFilePath, tempFiles[1]));
    } catch (e) {
      throw originalException;
    }
  }

  utils.run(COMMAND_ENCODE(tempFiles[1]), { cwd: outputPath });
  utils.run(COMMAND_CLEANUP(tempFiles[1], tempFiles[2]));
};

module.exports.FIX_FILE_PATH = FIX_FILE_PATH;
