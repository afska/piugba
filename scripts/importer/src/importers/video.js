const utils = require("../utils");
const $path = require("path");
const fs = require("fs");

const EXTENSION = "vid.bin";
const SIZE_PALETTE = 512;
const SIZE_TILES = 38912;

const COMMAND_RM_F = (path) => `rm -f "${path}"`;
const COMMAND_RM_RF = (dirPath) => `rm -rf "${dirPath}"`;
const COMMAND_MK_DIR = (dirPath) => `mkdir "${dirPath}"`;
const COMMAND_GET_FRAMES = (input, output) =>
  `ffmpeg -y -i "${input}" -r 30 "${output}/output_%05d.png"`;
const COMMAND_CONVERT = (
  baseFile,
  input,
  firstColorPalette,
  tempPalette,
  output
) =>
  // remap palette so the first color is black
  `magick "${input}" -resize ${RESOLUTION} -colors ${COLORS} -unique-colors "${tempPalette}" && ` +
  `magick "${firstColorPalette}" "${tempPalette}" +append "${tempPalette}" && ` +
  `magick "${input}" -resize ${RESOLUTION} -colors ${COLORS} -remap "${tempPalette}" "${output}" && ` +
  `rm "${tempPalette}" && ` +
  // encode the frame for mode 0 rendering
  `grit "${output}" -gt -gB8 -mRtf -mLs -ftb && ` +
  // ensure the palette is 512 bytes
  `truncate -s ${SIZE_PALETTE} "${baseFile}.pal.bin" &&` +
  // ensure the tileset is 38912 bytes
  `truncate -s ${SIZE_TILES} "${baseFile}.img.bin"`;
const RESOLUTION = "240x160!";
const COLORS = "253";
const EXTENSIONS_TMP = ["pal.bmp", "bmp", "h"];

const COMMAND_APPEND = (input1, input2, input3, output) =>
  `cat "${input1}" "${input2}" "${input3}" >> "${output}"`;

module.exports = async (outputName, filePath, outputPath, transparentColor) => {
  let fastSetting = GLOBAL_OPTIONS.fast;
  try {
    GLOBAL_OPTIONS.fast = true;
    const tempPath = `${filePath}_tmp`;

    await utils.run(COMMAND_RM_RF(tempPath));
    await utils.run(COMMAND_MK_DIR(tempPath));

    console.log(`  ⏳  Extracting video frames (${outputName})...`);
    await utils.run(COMMAND_GET_FRAMES(filePath, tempPath));
    const frames = fs.readdirSync(tempPath).sort();

    console.log(`  ⏳  Converting video frames (${outputName})...`);
    let count = 0;
    await utils.processContent(frames, async (frame) => {
      const name = $path.parse(frame).name;
      const baseFile = $path.join(tempPath, name);
      const frameFile = $path.join(tempPath, frame);
      const tempFiles = EXTENSIONS_TMP.map((it) =>
        $path.join(tempPath, `${name}.${it}`)
      );

      // convert frame
      await utils.run(
        COMMAND_CONVERT(
          baseFile,
          frameFile,
          transparentColor,
          tempFiles[0],
          tempFiles[1]
        ),
        { cwd: tempPath }
      );

      count++;
      console.log(
        `  ⏳  (${outputName}) | converted ${count} / ${frames.length}`
      );
    });

    console.log(`  ⏳  Adding video frames ${outputName}...`);
    const outputFilePath = $path.join(outputPath, `${outputName}.${EXTENSION}`);
    await utils.run(COMMAND_RM_F(outputFilePath));
    for (let frame of frames) {
      const name = $path.parse(frame).name;
      const baseFile = $path.join(tempPath, name);

      await utils.run(
        COMMAND_APPEND(
          `${baseFile}.pal.bin`,
          `${baseFile}.map.bin`,
          `${baseFile}.img.bin`,
          outputFilePath
        )
      );
    }

    await utils.run(COMMAND_RM_RF(tempPath));
  } finally {
    GLOBAL_OPTIONS.fast = fastSetting;
  }
};
