const Channels = require("./parser/Channels");
const importers = require("./importers");
const fs = require("fs");
const mkdirp = require("mkdirp");
const $path = require("path");
const utils = require("./utils");
const { printTable } = require("console-table-printer");
const getopt = require("node-getopt");
const _ = require("lodash");
require("colors");

const REMOVE_EXTENSION = (it) => it.replace(/\.[^/.]+$/, "");

const CONTENT_PATH = $path.join(__dirname, "../../../src/data/content");
const SONGS_PATH = $path.join(CONTENT_PATH, "songs");
const AUDIO_PATH = $path.join(CONTENT_PATH, "assets/audio");
const IMAGES_PATH = $path.join(CONTENT_PATH, "assets/images");
const OUTPUT_PATH = $path.join(CONTENT_PATH, "_compiled_files");

const SEPARATOR = " - ";
const MAX_FILE_LENGTH = 15;
const SELECTOR_OPTIONS = 4;
const FILE_METADATA = /\.ssc/i;
const FILE_AUDIO = /\.mp3/i;
const FILE_BACKGROUND = /\.png/i;
const MODE_OPTIONS = ["auto", "manual"];
const MODE_DEFAULT = "manual";

const opt = getopt
  .create([
    ["d", "mode=MODE", "how to complete missing data (one of: auto|*manual*)"],
    ["j", "json", "generate JSON debug files"],
    ["f", "force", "ignore errors"],
    ["a", "all", "include all charts, including NUMERIC difficulty levels"],
  ])
  .bindHelp()
  .parseSystem();

global.GLOBAL_OPTIONS = opt.options;
if (!_.includes(MODE_OPTIONS, GLOBAL_OPTIONS.mode))
  GLOBAL_OPTIONS.mode = MODE_DEFAULT;

const GET_SONG_FILES = ({ path, id, name }) => {
  const files = fs.readdirSync(path).map((it) => $path.join(path, it));

  return {
    metadataFile: _.find(files, (it) => FILE_METADATA.test(it)),
    audioFile: _.find(files, (it) => FILE_AUDIO.test(it)),
    backgroundFile: _.find(files, (it) => FILE_BACKGROUND.test(it)),
  };
};

// ---

mkdirp(SONGS_PATH);
mkdirp(AUDIO_PATH);
mkdirp(IMAGES_PATH);
utils.run(`rm -rf ${OUTPUT_PATH}`);
mkdirp.sync(OUTPUT_PATH);

const songs = _(fs.readdirSync(SONGS_PATH))
  .sortBy()
  .map((directory, i) => {
    const parts = directory.split(SEPARATOR);

    const id = parts.length === 2 ? _.first(parts) : i.toString();
    const name = _.last(parts);
    const outputName = (id + "-" + name)
      .replace(/[^0-9a-z -]/gi, "")
      .substring(0, MAX_FILE_LENGTH);

    return {
      path: $path.join(SONGS_PATH, directory),
      id,
      name,
      outputName,
    };
  })
  .sortBy("outputName")
  .value();

console.log(`${"Importing".bold} audio fx...`);
fs.readdirSync(AUDIO_PATH).forEach((audioFile) => {
  const name = `_aud_${REMOVE_EXTENSION(audioFile)}`;
  const path = $path.join(AUDIO_PATH, audioFile);

  utils.report(() => importers.audio(name, path, OUTPUT_PATH), audioFile);
});

let lastSelectorBuilt = -1;
const simfiles = songs.map((song, i) => {
  const { outputName } = song;
  const { metadataFile, audioFile, backgroundFile } = GET_SONG_FILES(song);

  console.log(
    `(${(i + 1).toString().red}${"/".red}${songs.length.toString().red}) ${
      "Importing".bold
    } ${song.name.cyan}...`
  );

  // metadata
  const simfile = utils.report(
    () => importers.metadata(outputName, metadataFile, OUTPUT_PATH),
    "charts"
  );

  // audio
  utils.report(
    () => importers.audio(outputName, audioFile, OUTPUT_PATH),
    "audio"
  );

  // background
  utils.report(
    () => importers.background(outputName, backgroundFile, OUTPUT_PATH),
    "background"
  );

  // selector
  let options = [];
  if ((i + 1) % SELECTOR_OPTIONS === 0 || i === songs.length - 1) {
    const from = lastSelectorBuilt + 1;
    const to = i;
    options = _.range(from, to + 1).map((j) => {
      return { song: songs[j], files: GET_SONG_FILES(songs[j]) };
    });
    lastSelectorBuilt = i;

    const name = `_sel_${from}`;
    utils.report(
      () => importers.selector(name, options, OUTPUT_PATH),
      "selector"
    );
  }

  return simfile;
});

printTable(
  simfiles.map((it) => ({
    id: it.metadata.id,
    title: it.metadata.title,
    artist: it.metadata.artist,
    channel: it.metadata.channel,
    normal: _.find(it.charts, (chart) => chart.header.difficulty === "NORMAL")
      .header.level,
    hard: _.find(it.charts, (chart) => chart.header.difficulty === "HARD")
      .header.level,
    crazy: _.find(it.charts, (chart) => chart.header.difficulty === "CRAZY")
      .header.level,
  }))
);

_.forEach(Channels, (v, k) => {
  const count = _.sumBy(simfiles, (it) => (it.metadata.channel === k ? 1 : 0));
  console.log(`${k}: `.bold + count.toString().cyan);
});
