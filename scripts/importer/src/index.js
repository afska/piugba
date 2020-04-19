const importers = require("./importers");
const fs = require("fs");
const mkdirp = require("mkdirp");
const $path = require("path");
const utils = require("./utils");
const _ = require("lodash");
require("colors");

const SONGS_PATH = `${__dirname}/../../../src/data/content/songs`;
const OUTPUT_PATH = `${SONGS_PATH}/../#compiled_songs`;

const FILE_METADATA = /\.ssc/i;
const FILE_AUDIO = /\.mp3/i;
const FILE_BACKGROUND = /\.png/i;

mkdirp(SONGS_PATH);
utils.run(`rm -rf ${OUTPUT_PATH}`);
mkdirp.sync(OUTPUT_PATH);

fs.readdirSync(SONGS_PATH)
  .map((directory) => {
    return {
      path: $path.join(SONGS_PATH, directory),
      name: _(directory).split(" - ").last(),
    };
  })
  .forEach(({ path, name }) => {
    const files = fs.readdirSync(path).map((it) => $path.join(path, it));
    const metadataFile = _.find(files, (it) => FILE_METADATA.test(it));
    const audioFile = _.find(files, (it) => FILE_AUDIO.test(it));
    const backgroundFile = _.find(files, (it) => FILE_BACKGROUND.test(it));
    console.log(`${"Importing".bold} ${name.cyan}...`);

    // metadata
    utils.report(
      () => importers.metadata(name, metadataFile, OUTPUT_PATH),
      "metadata"
    );

    // audio
    utils.report(() => importers.audio(name, audioFile, OUTPUT_PATH), "audio");

    // background
    utils.report(
      () => importers.background(name, backgroundFile, OUTPUT_PATH),
      "background"
    );
  });
