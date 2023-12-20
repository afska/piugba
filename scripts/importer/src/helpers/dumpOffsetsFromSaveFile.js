const importers = require("../importers");
const fs = require("fs");
const $path = require("path");
const _ = require("lodash");
require("colors");

// This script takes a song library and a save file with corrected custom offsets, and dumps the offsets using the `offset.pofs` format.

const OFFSET_TABLE_START_BYTE = 0x29b1;
const ARCADE_MAX_LEVELS = 100;
const ARCADE_MAX_SONGS = 100;
const TABLE_SIZE = ARCADE_MAX_LEVELS * ARCADE_MAX_SONGS;

const BASE_DIRECTORY = "./src/data/content/songs";
const SAVE_FILE = `${BASE_DIRECTORY}/offsets.sav`;

global.GLOBAL_OPTIONS = {
  mode: "auto",
  arcade: true,
  directory: BASE_DIRECTORY,
  boss: true,
};
const FILE_METADATA = /\.ssc$/i;

const GET_SONG_FILES = ({ path, name }) => {
  const files = fs.readdirSync(path).map((it) => $path.join(path, it));

  return {
    metadataFile: _.find(files, (it) => FILE_METADATA.test(it)),
  };
};

function read(packPath) {
  return _(
    fs.readdirSync(packPath, {
      withFileTypes: true,
    })
  )
    .sortBy()
    .filter((it) => it.isDirectory())
    .map("name")
    .map((directory) => {
      const name = directory;

      return {
        path: $path.join(packPath, directory),
        name,
      };
    })
    .sortBy("outputName")
    .map((it, id) => ({ ...it, id }))
    .value();
}

function importSong(song) {
  const { id, outputName } = song;
  const { metadataFile } = GET_SONG_FILES(song);

  return importers.metadata(outputName, metadataFile, ".", id);
}

const savefile = fs.readFileSync(SAVE_FILE);
const s = read(BASE_DIRECTORY);
const songs = s.map((song) => {
  return importSong(song);
});

const print = (song, charts, ctx) => {
  charts.forEach((it, i) => {
    const offset = savefile.readInt8(
      OFFSET_TABLE_START_BYTE +
        it.header.isDouble * TABLE_SIZE +
        song.id * ARCADE_MAX_LEVELS +
        i
    );

    if ((ctx.onlyZero && offset === 0) || (!ctx.onlyZero && offset !== 0)) {
      const correction = `${it.metadata.title}[${it.header.levelStr}]=${offset}`.replace(
        /\[(\d)\]\]/g,
        "][$1]"
      );
      console.log(correction);
      ctx.hasCorrections = true;
    }
  });
};

songs.forEach((song) => {
  const ctx = { hasCorrections: false };

  const single = song.charts.filter((it) => !it.header.isDouble);
  const double = song.charts.filter(
    (it) => it.header.isDouble && !it.header.isMultiplayer
  );
  const multiplayer = song.charts.filter(
    (it) => it.header.isDouble && it.header.isMultiplayer
  );

  print(song, single, ctx);
  print(song, double, ctx);
  print(song, multiplayer, ctx);

  if (ctx.hasCorrections) {
    ctx.onlyZero = true;
    print(song, single, ctx);
    print(song, double, ctx);
    print(song, multiplayer, ctx);
  }
});
