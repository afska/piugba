const utils = require("../utils");
const $path = require("path");
const fs = require("fs");
const fsAsync = require("fs/promises");
const _ = require("lodash");

const CONVERT_CONCURRENCY = 10;
const EXTENSION = "vid.bin";

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
  `truncate -s 512 "${baseFile}.pal.bin" &&` +
  // ensure the tileset is... well...
  `bash -c 'fileSize=$(stat -c "%s" "${baseFile}.img.bin"); ` +
  `if [ $fileSize -gt 38912 ]; then ` + // if it's greater than 38912, truncate down
  `newSize=38912; ` +
  `else ` + // if it's smaller, pad to the next 512-byte boundary
  `newSize=$(( ($fileSize + 511) / 512 * 512 )); ` +
  `fi; ` +
  `truncate -s $newSize "${baseFile}.img.bin"'`;
const RESOLUTION = "240x160!";
const COLORS = "253";
const EXTENSIONS_TMP = ["pal.bmp", "bmp", "h"];

const COMMAND_APPEND = (input1, input2, input3, output) =>
  `cat "${input1}" "${input2}" "${input3}" >> "${output}"`;

module.exports = async (outputName, filePath, outputPath, transparentColor) => {
  const tempPath = `${filePath}_tmp`;

  await utils.run(COMMAND_RM_RF(tempPath));
  await utils.run(COMMAND_MK_DIR(tempPath));

  console.log(`  ⏳  Extracting video frames (${outputName})...`);
  await utils.run(COMMAND_GET_FRAMES(filePath, tempPath));
  const frames = fs.readdirSync(tempPath).sort();

  console.log(`  ⏳  Converting video frames (${outputName})...`);
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

      // add tileset size (in sectors) to last byte of palette
      const palFile = `${baseFile}.pal.bin`;
      const imgFile = `${baseFile}.img.bin`;
      const stats = await fsAsync.stat(imgFile);
      const fileSize = stats.size;
      const sectors = Math.floor(fileSize / 512);
      const sectorByte = Buffer.from([sectors]);
      const fd = await fsAsync.open(palFile, "r+");
      await fd.write(sectorByte, 0, sectorByte.length, 511);
      await fd.close();

      count++;
      console.log(
        `  ⏳  (${outputName}) | converted ${count} / ${frames.length}`
      );
    });
  }

  console.log(`  ⏳  Adding video frames ${outputName}...`);
  for (let frame of frames) {
    const name = $path.parse(frame).name;
    const baseFile = $path.join(tempPath, name);

    await utils.run(
      COMMAND_APPEND(
        `${baseFile}.pal.bin`,
        `${baseFile}.map.bin`,
        `${baseFile}.img.bin`,
        $path.join(outputPath, `${outputName}.${EXTENSION}`)
      )
    );
  }

  await utils.run(COMMAND_RM_RF(tempPath));
};
