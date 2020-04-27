const buildSelector = require("./selector/buildSelector");
const utils = require("../utils");
const $path = require("path");
const $tmp = require("tmp");
const fs = require("fs");
const background = require("./background");

const SELECTOR_DIR = "selector";
const SELECTOR_MSL = "selector.msl";
const SELECTOR_BMP = "selector.bmp";
const SELECTOR_OUTPUT_PNG = "output.bmp";
const COMMAND_1 = "magick conjure msl:selector.msl || echo Done!";
const COMMAND_2 = (input) => `rm -rf "${input}"`;

module.exports = (name, options, outputPath) => {
  const tmpDir = $tmp.dirSync();

  fs.writeFileSync(
    $path.join(tmpDir.name, SELECTOR_MSL),
    buildSelector(options.length)
  );
  fs.copyFileSync(
    $path.join(__dirname, SELECTOR_DIR, SELECTOR_BMP),
    $path.join(tmpDir.name, SELECTOR_BMP)
  );
  options.forEach(({ files }, i) => {
    fs.copyFileSync(
      files.backgroundFile,
      $path.join(tmpDir.name, `song${i + 1}.png`)
    );
  });

  const outputImage = $path.join(tmpDir.name, `${name}.bmp`);
  utils.run(COMMAND_1, { cwd: tmpDir.name });
  fs.renameSync($path.join(tmpDir.name, SELECTOR_OUTPUT_PNG), outputImage);
  background(name, outputImage, outputPath);
  utils.run(COMMAND_2(tmpDir.name));
};
