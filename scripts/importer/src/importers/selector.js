const buildSelector = require("./selector/buildSelector");
const { FIX_FILE_PATH } = require("./background");
const utils = require("../utils");
const $path = require("path");
const $tmp = require("tmp");
const fs = require("fs");

const SELECTOR_MSL = "selector.msl";
const SELECTOR_BMP = "selector.bmp";
const SELECTOR_OUTPUT_PNG = "output.bmp";
const COMMAND_1 = "magick conjure msl:selector.msl || echo Done!";
const COMMAND_2 = (input) => `grit "${input}" -gt -gB8 -mRtf -mLs -ftb`;
const COMMAND_3 = (input) => `rm -rf "${input}"`;
const EXTENSION_TMP = "h";

module.exports = (name, options, outputPath, selectorDir) => {
  const tempDir = $tmp.dirSync();
  const tempFile = $path.join(outputPath, `${name}.${EXTENSION_TMP}`);

  fs.writeFileSync(
    $path.join(tempDir.name, SELECTOR_MSL),
    buildSelector(options.length)
  );
  fs.copyFileSync(
    $path.join(selectorDir, SELECTOR_BMP),
    $path.join(tempDir.name, SELECTOR_BMP)
  );
  options.forEach(({ files }, i) => {
    try {
      fs.copyFileSync(
        FIX_FILE_PATH(files.backgroundFile),
        $path.join(tempDir.name, `song${i + 1}.png`)
      );
    } catch (e) {
      fs.copyFileSync(
        files.backgroundFile,
        $path.join(tempDir.name, `song${i + 1}.png`)
      );
    }
  });

  const outputImage = $path.join(tempDir.name, `${name}.bmp`);
  utils.run(COMMAND_1, { cwd: tempDir.name });
  fs.renameSync($path.join(tempDir.name, SELECTOR_OUTPUT_PNG), outputImage);
  utils.run(COMMAND_2(outputImage), { cwd: outputPath });

  utils.run(COMMAND_3(tempFile));
  utils.run(COMMAND_3(tempDir.name));
};
