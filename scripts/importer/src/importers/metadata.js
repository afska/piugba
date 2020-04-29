const Simfile = require("../parser/Simfile");
const Channels = require("../parser/Channels");
const SongSerializer = require("../serializer/SongSerializer");
const fs = require("fs");
const $path = require("path");
const utils = require("../utils");
const _ = require("lodash");

const EXTENSION = "pius";

module.exports = (name, filePath, outputPath) => {
  const content = fs.readFileSync(filePath).toString();
  const { metadata, charts } = new Simfile(content, name);

  if (_.isEmpty(charts)) throw new Error("no_charts");
  completeMissingInformation(metadata, charts);

  const simfile = { metadata, charts };
  const output = new SongSerializer(simfile).serialize();

  fs.writeFileSync($path.join(outputPath, `${name}.${EXTENSION}`), output);
};

const completeMissingInformation = (metadata, charts) => {
  if (metadata.channel === "UNKNOWN") {
    console.log(
      "-> channels: ".bold + `(0 = ORIGINAL, 1 = KPOP, 2 = WORLD)`.cyan
    );
    const channel = utils.insistentChoice("What channel?", _.range(3));
    metadata.channel = _.keys(Channels)[parseInt(channel)];
  }

  setDifficulty(charts, "NORMAL");
  setDifficulty(charts, "HARD");
  setDifficulty(charts, "CRAZY");

  const writeChanges = utils.insistentChoice(
    "Write simfile? (y/n)",
    ["y", "n"],
    "red"
  );
};

const setDifficulty = (charts, difficultyName) => {
  const numericDifficultyCharts = charts.filter(
    (it) => it.header.difficulty === "NUMERIC"
  );
  const levels = numericDifficultyCharts.map(
    (it) => `${it.header.name}[${it.header.level}]`
  );
  const levelNames = numericDifficultyCharts.map((it) => it.header.name);

  if (!_.some(charts, { difficulty: difficultyName })) {
    console.log("-> levels: ".bold + `(${levels.join(", ")})`.cyan);
    const chartName = utils.insistentChoice(
      `Which one is ${difficultyName}?`,
      levelNames
    );

    _.find(
      charts,
      (it) => it.header.name.toLowerCase() === chartName
    ).header.difficulty = difficultyName;
  }
};
