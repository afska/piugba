const Simfile = require("../parser/Simfile");
const Channels = require("../parser/Channels");
const SongSerializer = require("../serializer/SongSerializer");
const fs = require("fs");
const $path = require("path");
const utils = require("../utils");
const _ = require("lodash");

const EXTENSION = "pius";
const KNOWN_CHANNELS = ["ORIGINAL", "KPOP", "WORLD"];
const NON_NUMERIC_LEVELS = ["NORMAL", "HARD", "CRAZY"];

module.exports = (name, filePath, outputPath) => {
  const content = fs.readFileSync(filePath).toString();
  const { metadata, charts } = new Simfile(content, name);

  checkIntegrity(metadata, charts);
  completeMissingInformation(metadata, charts);
  const selectedCharts = charts.filter((it) =>
    _.includes(NON_NUMERIC_LEVELS, it.header.difficulty)
  );

  const simfile = { metadata, charts: selectedCharts };
  const output = new SongSerializer(simfile).serialize();

  fs.writeFileSync($path.join(outputPath, `${name}.${EXTENSION}`), output);
};

const checkIntegrity = (metadata, charts) => {
  if (_.isEmpty(metadata.id)) throw new Error("missing_id");
  if (_.isEmpty(metadata.title)) throw new Error("missing_title");
  if (_.isEmpty(metadata.artist)) throw new Error("missing_artist");
  if (!_.isFinite(metadata.sampleStart)) throw new Error("invalid_samplestart");
  if (!_.isFinite(metadata.sampleLength))
    throw new Error("invalid_samplelength");
  if (_.isEmpty(charts)) throw new Error("no_charts");
};

const completeMissingInformation = (metadata, charts) => {
  if (metadata.channel === "UNKNOWN") {
    const channelOptions = KNOWN_CHANNELS.map(
      (name, i) => `${i} = ${name}`
    ).join(", ");

    console.log("-> channels: ".bold + `(${channelOptions})`.cyan);
    const channel = utils.insistentChoice(
      "What channel?",
      _.range(KNOWN_CHANNELS.length)
    );
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

  return;
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
