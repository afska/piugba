const utils = require("../utils");
const $path = require("path");
const fs = require("fs");
const _ = require("lodash");

const CONVERT_CONCURRENCY = 10;

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
  `magick "${input}" -resize ${RESOLUTION} -colors ${COLORS} -unique-colors "${tempPalette}" && ` +
  `magick "${firstColorPalette}" "${tempPalette}" +append "${tempPalette}" && ` +
  `magick "${input}" -resize ${RESOLUTION} -colors ${COLORS} -remap "${tempPalette}" "${output}" && ` +
  `rm "${tempPalette}" && ` +
  `grit "${output}" -gt -gB8 -mRtf -mLs -ftb && ` +
  `truncate -s 512 "${baseFile}.pal.bin" && ` +
  `truncate -s 38912 "${baseFile}.img.bin"`; // TODO: FIX LATER - ADD SIZE HEADER
const RESOLUTION = "240x160!";
const COLORS = "253";
const EXTENSIONS_TMP = ["pal.bmp", "bmp", "h"];

const COMMAND_APPEND = (input1, input2, input3, output) =>
  `cat "${input1}" "${input2}" "${input3}" >> "${output}"`;

const VIDEO_LIB_METADATA_SIZE = 1024;
const VIDEO_LIB_HEADER_SIZE = 40;
const VIDEO_LIB_ENTRY_SIZE = 8;

module.exports = async (id, filePath, videoLibFile, transparentColor) => {
  const tempPath = `${filePath}_tmp`;

  await utils.run(COMMAND_RM_RF(tempPath));
  await utils.run(COMMAND_MK_DIR(tempPath));

  console.log(`  ⏳  Extracting video frames (${id})...`);
  await utils.run(COMMAND_GET_FRAMES(filePath, tempPath));
  const frames = fs.readdirSync(tempPath).sort();
  const videoContentOffset = fs.statSync(videoLibFile).size;

  console.log(`  ⏳  Converting video frames (${id})...`);
  let count = 0;
  const chunks = _.chunk(frames, CONVERT_CONCURRENCY);
  for (let chunk of chunks) {
    await utils.processContent(chunk, async (frame) => {
      const name = $path.parse(frame).name;
      const baseFile = $path.join(tempPath, name);
      const frameFile = $path.join(tempPath, frame);
      const tempFiles = EXTENSIONS_TMP.map((it) =>
        $path.join(tempPath, `${name}.${it}`)
      );

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
      console.log(`  ⏳  (${id}) | converted ${count} / ${frames.length}`);
    });
  }

  console.log(`  ⏳  Adding video frames ${id}...`);
  for (let frame of frames) {
    const name = $path.parse(frame).name;
    const baseFile = $path.join(tempPath, name);

    await utils.run(
      COMMAND_APPEND(
        `${baseFile}.pal.bin`,
        `${baseFile}.map.bin`,
        `${baseFile}.img.bin`,
        videoLibFile
      )
    );
  }

  await utils.run(COMMAND_RM_RF(tempPath));

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
