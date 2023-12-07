const importers = require("./importers");
const fs = require("fs");
const $path = require("path");
const _ = require("lodash");
require("colors");

const DIRECTORY = "./src/data/content/songs-pack/(0) Pump It Up!/";
const BASE_DIRECTORY = "./src/data/content/songs-pack-base/(0) Pump It Up!/";

global.GLOBAL_OPTIONS = {
  mode: "auto",
  arcade: false,
  directory: DIRECTORY,
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

const s1 = read(DIRECTORY);
const s2 = read(BASE_DIRECTORY);

const songs1 = s1.map((song) => {
  return importSong(song);
});
const songs2 = s2.map((song) => {
  return importSong(song);
});

songs1.forEach((song, i) => {
  if (songs2[i].charts.length === song.charts.length);
  else console.log(song.metadata.title);
});
