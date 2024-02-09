const utils = require("../utils");
const $path = require("path");
const fs = require("fs");

const COMMAND_RM_RF = (dirPath) => `rm -rf "${dirPath}"`;
const COMMAND_MK_DIR = (dirPath) => `mkdir "${dirPath}"`;
const COMMAND_GET_FRAMES = (input, output) =>
  `ffmpeg -y -i "${input}" -r 30 "${output}/output_%05d.png"`;

const COMMAND_BUILD_REMAP = (input, firstColorPalette, tempPalette, output) =>
  `magick "${input}" -resize ${RESOLUTION} -colors ${COLORS} -unique-colors "${tempPalette}" && ` +
  `magick "${firstColorPalette}" "${tempPalette}" +append "${tempPalette}" && ` +
  `magick "${input}" -resize ${RESOLUTION} -colors ${COLORS} -remap "${tempPalette}" "${output}" && ` +
  `rm "${tempPalette}"`;
const COMMAND_ENCODE = (input) => `grit "${input}" -gt -gB8 -mRtf -mLs -ftb`;
const COMMAND_PAD_PAL = (input) => `truncate -s 512 "${input}`;
const COMMAND_PAD = (input) => `truncate -s 38464 "${input}`; // TODO: INCORRECT! FIX LATER - ADD SIZE HEADER
const RESOLUTION = "240x160!";
const COLORS = "253";
const EXTENSIONS_TMP = ["pal.bmp", "bmp", "h"];

const COMMAND_APPEND = (input1, input2, input3, output) =>
  `cat "${input1}" "${input2}" "${input3}" >> "${output}"`;

const VIDEO_LIB_METADATA_SIZE = 840;
const VIDEO_LIB_HEADER_SIZE = 40;
const VIDEO_LIB_ENTRY_SIZE = 8;

module.exports = (id, filePath, videoLibFile, transparentColor) => {
  const tempPath = `${filePath}_tmp`;

  utils.run(COMMAND_RM_RF(tempPath));
  utils.run(COMMAND_MK_DIR(tempPath));
  utils.run(COMMAND_GET_FRAMES(filePath, tempPath));

  const videoContentOffset = fs.statSync(videoLibFile).size;

  const frames = fs.readdirSync(tempPath);
  let i = 0;
  for (let frame of frames) {
    const name = $path.parse(frame).name;
    const tempFiles = EXTENSIONS_TMP.map((it) =>
      $path.join(tempPath, `${name}.${it}`)
    );

    const baseFile = $path.join(tempPath, name);
    const frameFile = $path.join(tempPath, frame);

    console.log(`  ⏳  (${id}) | frame ${++i} / ${frames.length}`);

    utils.run(
      COMMAND_BUILD_REMAP(
        frameFile,
        transparentColor,
        tempFiles[0],
        tempFiles[1]
      )
    );
    utils.run(COMMAND_ENCODE(tempFiles[1]), { cwd: tempPath });
    utils.run(COMMAND_PAD_PAL(`${baseFile}.pal.bin`));
    utils.run(COMMAND_PAD(`${baseFile}.img.bin`));
    utils.run(
      COMMAND_APPEND(
        `${baseFile}.pal.bin`,
        `${baseFile}.map.bin`,
        `${baseFile}.img.bin`,
        videoLibFile
      )
    );
  }

  utils.run(COMMAND_RM_RF(tempPath));

  const videoEntryOffset = VIDEO_LIB_HEADER_SIZE + VIDEO_LIB_ENTRY_SIZE * id;
  const videoEntry = Buffer.alloc(VIDEO_LIB_ENTRY_SIZE);
  videoEntry.writeUInt32LE(frames.length);
  videoEntry.writeUInt32LE(videoContentOffset, 4);
  const lib = fs.openSync(videoLibFile, "r+");
  fs.writeSync(lib, videoEntry, 0, VIDEO_LIB_ENTRY_SIZE, videoEntryOffset);
  fs.closeSync(lib);
};

module.exports.VIDEO_LIB_METADATA_SIZE = VIDEO_LIB_METADATA_SIZE;
module.exports.VIDEO_LIB_HEADER_SIZE = VIDEO_LIB_HEADER_SIZE;
