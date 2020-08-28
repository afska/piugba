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

const CREATE_ID = (i) => _.padStart(i, 3, "0");
const ROM_ID_FILE = "_rom_id.u32";
const NORMALIZE_FILENAME = (it, prefix = "") =>
  prefix +
  it.replace(/[^0-9a-z -]/gi, "").substring(0, MAX_FILE_LENGTH - prefix.length);
const REMOVE_EXTENSION = (it) => it.replace(/\.[^/.]+$/, "");

const DATA_PATH = $path.join(__dirname, "../../../src/data");
const CONTENT_PATH = $path.join(DATA_PATH, "content");
const AUDIO_PATH = $path.join(DATA_PATH, "assets/audio");
const IMAGES_PATH = $path.join(DATA_PATH, "assets/images");
const SONGS_PATH = $path.join(CONTENT_PATH, "songs");
const OUTPUT_PATH = $path.join(CONTENT_PATH, "_compiled_files");

const MAX_FILE_LENGTH = 15;
const MAX_SONGS = 99;
const SELECTOR_OPTIONS = 4;
const FILE_METADATA = /\.ssc$/i;
const FILE_AUDIO = /\.(mp3|flac)$/i;
const FILE_BACKGROUND = /\.png$/i;
const MODE_OPTIONS = ["auto", "manual"];
const MODE_DEFAULT = "manual";

// ------------
// COMMAND LINE
// ------------

const opt = getopt
  .create([
    ["d", "mode=MODE", "how to complete missing data (one of: auto|*manual*)"],
    ["j", "json", "generate JSON debug files"],
    ["a", "all", "include all charts, including NUMERIC difficulty levels"],
  ])
  .bindHelp()
  .parseSystem();

global.GLOBAL_OPTIONS = opt.options;
if (!_.includes(MODE_OPTIONS, GLOBAL_OPTIONS.mode))
  GLOBAL_OPTIONS.mode = MODE_DEFAULT;

const GET_SONG_FILES = ({ path, name }) => {
  const files = fs.readdirSync(path).map((it) => $path.join(path, it));

  return {
    metadataFile: _.find(files, (it) => FILE_METADATA.test(it)),
    audioFile: _.find(files, (it) => FILE_AUDIO.test(it)),
    backgroundFile: _.find(files, (it) => FILE_BACKGROUND.test(it)),
  };
};

// -------
// CLEANUP
// -------

mkdirp(SONGS_PATH);
utils.run(`rm -rf ${OUTPUT_PATH}`);
mkdirp.sync(OUTPUT_PATH);

// ------------
// AUDIO ASSETS
// ------------

console.log(`${"Importing".bold} audio...`);
fs.readdirSync(AUDIO_PATH).forEach((audioFile) => {
  if (!FILE_AUDIO.test(audioFile)) return;

  const name = NORMALIZE_FILENAME(REMOVE_EXTENSION(audioFile), "_aud_");
  const path = $path.join(AUDIO_PATH, audioFile);

  utils.report(() => importers.audio(name, path, OUTPUT_PATH), audioFile);
});

// ------------
// IMAGE ASSETS
// ------------

console.log(`${"Importing".bold} images...`);
fs.readdirSync(IMAGES_PATH).forEach((imageFile) => {
  if (!FILE_BACKGROUND.test(imageFile)) return;

  const name = NORMALIZE_FILENAME(REMOVE_EXTENSION(imageFile), "_img_");
  const path = $path.join(IMAGES_PATH, imageFile);

  utils.report(() => importers.background(name, path, OUTPUT_PATH), imageFile);
});

// ---------------
// SONGS DETECTION
// ---------------

const songs = _(fs.readdirSync(SONGS_PATH))
  .sortBy()
  .map((directory) => {
    const name = directory;
    const outputName = NORMALIZE_FILENAME(name);

    return {
      path: $path.join(SONGS_PATH, directory),
      name,
      outputName,
    };
  })
  .sortBy("outputName")
  .value();

// ------------
// SONGS IMPORT
// ------------

if (songs.length > MAX_SONGS) throw new Error("song_limit_reached");

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
      () => importers.selector(name, options, OUTPUT_PATH, IMAGES_PATH),
      "selector"
    );
  }

  return simfile;
});

// -------
// ROM ID
// -------

const romIdBuffer = Buffer.alloc(4);
romIdBuffer.writeUInt32LE(Math.random() * 0xffffffff);
romIdBuffer.writeUInt8(songs.length, 3);
fs.writeFileSync($path.join(OUTPUT_PATH, ROM_ID_FILE), romIdBuffer);

// -------
// SUMMARY
// -------

printTable(
  simfiles.map((it, i) => ({
    id: CREATE_ID(i),
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
