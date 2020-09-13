const Channels = require("./parser/Channels");
const importers = require("./importers");
const fs = require("fs");
const mkdirp = require("mkdirp");
const $path = require("path");
const utils = require("./utils");
const getopt = require("node-getopt");
const _ = require("lodash");
require("colors");

const CREATE_ID = (i) => _.padStart(i, ID_SIZE, "0");
const ROM_ID_FILE = "_rom_id.u32";
const ROM_ID_FILE_REUSE = "romid.u32";
const ROM_NAME_FILE = "_rom_name.txt";
const ROM_NAME_FILE_SOURCE = "romname.txt";
const NORMALIZE_FILENAME = (it, prefix = "") =>
  prefix +
  it.replace(/[^0-9a-z -]/gi, "").substring(0, MAX_FILE_LENGTH - prefix.length);
const REMOVE_EXTENSION = (it) => it.replace(/\.[^/.]+$/, "");

const DATA_PATH = $path.join(__dirname, "../../../src/data");
const CONTENT_PATH = $path.join(DATA_PATH, "content");
const AUDIO_PATH = $path.join(DATA_PATH, "assets/audio");
const IMAGES_PATH = $path.join(DATA_PATH, "assets/images");
const BLACK_DOT_FILE = $path.join(IMAGES_PATH, "black.bmp");
const OUTPUT_PATH = $path.join(CONTENT_PATH, "_compiled_files");

const DEFAULT_SONGS_PATH = $path.join(CONTENT_PATH, "songs");

const ID_SIZE = 3;
const MAX_FILE_LENGTH = 15;
const MAX_SONGS = 99;
const SELECTOR_OPTIONS = 4;
const FILE_METADATA = /\.ssc$/i;
const FILE_AUDIO = /\.(mp3|flac)$/i;
const FILE_BACKGROUND = /\.png$/i;
const MODE_OPTIONS = ["auto", "manual"];
const MODE_DEFAULT = "auto";
const SORTING_OPTIONS = ["level", "dir"];
const SORTING_DEFAULT = "level";
const SELECTOR_PREFIXES = {
  NORMAL: "_snm_",
  HARD: "_shd_",
  CRAZY: "_scz_",
};
const LIBRARY_SUFFIX = "_list.txt";
const CAMPAIGN_LEVELS = _.keys(SELECTOR_PREFIXES);

// ------------
// COMMAND LINE
// ------------

const opt = getopt
  .create([
    [
      "d",
      "directory=DIRECTORY",
      "songs directory (defaults to: src/data/content/songs)",
    ],
    ["m", "mode=MODE", "how to complete missing data (one of: *auto*|manual)"],
    ["s", "sort=SORT", "how songs should be ordered (one of: *level*|dir)"],
    ["j", "json", "generate JSON debug files"],
    ["a", "all", "include all charts, including NUMERIC difficulty levels"],
  ])
  .bindHelp()
  .parseSystem();

global.GLOBAL_OPTIONS = opt.options;
if (!_.includes(MODE_OPTIONS, GLOBAL_OPTIONS.mode))
  GLOBAL_OPTIONS.mode = MODE_DEFAULT;
if (!_.includes(SORTING_OPTIONS, GLOBAL_OPTIONS.sort))
  GLOBAL_OPTIONS.sort = SORTING_DEFAULT;
const SONGS_PATH = opt.options.directory || DEFAULT_SONGS_PATH;

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
let reuseRomId = null;
try {
  reuseRomId = fs
    .readFileSync($path.join(SONGS_PATH, ROM_ID_FILE_REUSE))
    .readUInt32LE();
  console.log(
    `${"Reusing".bold.yellow} ${"rom id".yellow}: ${reuseRomId.toString().cyan}`
  );
  console.log(
    "To stop reusing rom ids, remove ".yellow +
      `${$path.join(SONGS_PATH, ROM_ID_FILE_REUSE)}`.cyan
  );
} catch (e) {}
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

// --------------
// SONG DETECTION
// --------------

const songs = _(fs.readdirSync(SONGS_PATH, { withFileTypes: true }))
  .sortBy()
  .filter((it) => it.isDirectory())
  .map("name")
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

// -----------
// SONG IMPORT
// -----------

if (songs.length > MAX_SONGS) throw new Error("song_limit_reached");

const processedSongs = songs.map((song, i) => {
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
    () =>
      importers.background(
        outputName,
        backgroundFile,
        OUTPUT_PATH,
        BLACK_DOT_FILE
      ),
    "background"
  );

  return { song, simfile };
});

// ------------
// SONG SORTING
// ------------

const sortedSongsByLevel = CAMPAIGN_LEVELS.map((difficultyLevel) => {
  const withIds = (songs) =>
    songs.map((it, i) => ({ ...it, id: CREATE_ID(i) }));

  return {
    difficultyLevel,
    songs: withIds(
      GLOBAL_OPTIONS.sort === "dir"
        ? processedSongs
        : _.orderBy(
            processedSongs,
            [
              ({ simfile }) => {
                const chart = simfile.getChartByDifficulty(difficultyLevel);
                return chart.header.order;
              },
              ({ simfile }) => {
                const chart = simfile.getChartByDifficulty(difficultyLevel);
                return _.sumBy(chart.events, "complexity");
              },
            ],
            ["ASC", "ASC"]
          )
    ),
  };
});

// ---------
// SELECTORS
// ---------

sortedSongsByLevel.forEach(({ difficultyLevel, songs }) => {
  let lastSelectorBuilt = -1;

  songs.forEach((___, i) => {
    if (i === 0)
      console.log(`${"Importing".bold} ${difficultyLevel.cyan} selectors...`);

    let options = [];
    if ((i + 1) % SELECTOR_OPTIONS === 0 || i === songs.length - 1) {
      const from = lastSelectorBuilt + 1;
      const to = i;
      options = _.range(from, to + 1).map((j) => {
        return {
          song: songs[j].song,
          files: GET_SONG_FILES(songs[j].song),
        };
      });

      const name = SELECTOR_PREFIXES[difficultyLevel] + from;
      const library =
        options.map(({ song }) => song.outputName).join("\r\n") + "\0";
      fs.writeFileSync($path.join(OUTPUT_PATH, name + LIBRARY_SUFFIX), library);

      lastSelectorBuilt = i;
      utils.report(
        () => importers.selector(name, options, OUTPUT_PATH, IMAGES_PATH),
        `[${from}-${to}]`
      );
    }
  });
});

// -------
// ROM ID
// -------

const romIdBuffer = Buffer.alloc(4);
if (reuseRomId !== null) romIdBuffer.writeUInt32LE(reuseRomId);
else {
  romIdBuffer.writeUInt32LE(Math.random() * 0xffffffff);
  romIdBuffer.writeUInt8(songs.length);
}
fs.writeFileSync($path.join(OUTPUT_PATH, ROM_ID_FILE), romIdBuffer);
fs.writeFileSync($path.join(SONGS_PATH, ROM_ID_FILE_REUSE), romIdBuffer);

// --------
// ROM NAME
// --------

let name = "";
try {
  const romName = fs
    .readFileSync($path.join(SONGS_PATH, ROM_NAME_FILE_SOURCE))
    .toString();
  name = _.padEnd(romName.substring(0, 12), 13, "\0");
} catch (e) {}
fs.writeFileSync($path.join(OUTPUT_PATH, ROM_NAME_FILE), name);

// ----------
// SONG LISTS
// ----------

sortedSongsByLevel.forEach(({ difficultyLevel, songs }) => {
  console.log(`\n${"SONG LIST".bold} - ${difficultyLevel.cyan}:`);

  utils.printTable(
    songs.map(({ simfile: it, id }) => {
      const normal = it.getChartByDifficulty("NORMAL");
      const hard = it.getChartByDifficulty("HARD");
      const crazy = it.getChartByDifficulty("CRAZY");

      const print = (n, digits) => _.padStart(n, digits, 0);

      const levelOf = (chart) => {
        const level = chart.header.level;
        const complexity = Math.round(
          _.sumBy(chart.events, "complexity") * 100
        );
        return `${print(level, 2)} (Ω ${print(complexity, 4)})`;
      };

      return {
        id,
        title: it.metadata.title,
        artist: it.metadata.artist,
        channel: it.metadata.channel,
        normal: levelOf(normal),
        hard: levelOf(hard),
        crazy: levelOf(crazy),
      };
    })
  );
});

// -------
// SUMMARY
// -------

console.log(`\n${"SUMMARY".bold}:\n`);

_.forEach(Channels, (v, k) => {
  const count = _.sumBy(processedSongs, ({ simfile: it }) =>
    it.metadata.channel === k ? 1 : 0
  );
  console.log(`${k}: `.bold + count.toString().cyan);
});
console.log("TOTAL: ".bold + processedSongs.length.toString().cyan);
