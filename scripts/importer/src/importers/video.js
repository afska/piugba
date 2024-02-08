const utils = require("../utils");
const $path = require("path");
const fs = require("fs");

const COMMAND_RM_DIR = (dirPath) => `rm -rf "${dirPath}"`;
const COMMAND_MK_DIR = (dirPath) => `mkdir "${dirPath}"`;
const COMMAND_GET_FRAMES = (input, output) =>
  `ffmpeg -y -i "${input}" -r 30 "${output}/output_%05d.png"`;
const COMMAND_ENCODE = (input) => `grit "${input}" -gt -gB1 -Mw2 -Mh2 -p -ftb`;
const COMMAND_APPEND = (input, output) => `cat "${input}" >> "${output}"`;

const VIDEO_LIB_METADATA_SIZE = 840;
const VIDEO_LIB_HEADER_SIZE = 40;
const VIDEO_LIB_ENTRY_SIZE = 8;

module.exports = (id, filePath, videoLibFile) => {
  const tempPath = `${filePath}_tmp`;

  utils.run(COMMAND_RM_DIR(tempPath));
  utils.run(COMMAND_MK_DIR(tempPath));
  utils.run(COMMAND_GET_FRAMES(filePath, tempPath));

  const videoContentOffset = fs.statSync(videoLibFile).size;

  const frames = fs.readdirSync(tempPath);
  let i = 0;
  for (let frame of frames) {
    const name = $path.parse(frame).name;
    const baseFile = $path.join(tempPath, name);
    const frameFile = $path.join(tempPath, frame);

    console.log(`  ⏳  (${id}) | frame ${++i} / ${frames.length}`);
    console.log(frameFile);
    utils.run(COMMAND_ENCODE(frameFile), { cwd: tempPath });
    utils.run(COMMAND_APPEND(`${baseFile}.pal.bin`, videoLibFile));
    utils.run(COMMAND_APPEND(`${baseFile}.img.bin`, videoLibFile));
  }

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
