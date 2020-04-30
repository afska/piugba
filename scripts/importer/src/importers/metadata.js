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
const GLOBAL_PROPERTY = (name) => new RegExp(`#${name}:((.|(\r|\n))*?);`, "g");

module.exports = (name, filePath, outputPath) => {
  const content = fs.readFileSync(filePath).toString();
  const { metadata, charts } = new Simfile(content, name);

  checkIntegrity(metadata, charts);
  completeMissingInformation(metadata, charts, content, filePath);
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

const completeMissingInformation = (metadata, charts, content, filePath) => {
  let isDirty = false;

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

    isDirty = true;
  }

  isDirty = isDirty || setDifficulty(charts, "NORMAL");
  isDirty = isDirty || setDifficulty(charts, "HARD");
  isDirty = isDirty || setDifficulty(charts, "CRAZY");

  if (isDirty) {
    const writeChanges = utils.insistentChoice(
      "Write simfile? (y/n)",
      ["y", "n"],
      "red"
    );
    if (writeChanges === "y") {
      let newContent = content.replace(
        GLOBAL_PROPERTY("GENRE"),
        `#GENRE:${metadata.channel};`
      );
      charts.forEach(({ header }) => {
        newContent = utils.replaceRange(
          newContent,
          GLOBAL_PROPERTY("DIFFICULTY"),
          `#DIFFICULTY:${header.difficulty};`,
          header.startIndex
        );
      });

      fs.writeFileSync(filePath, newContent);
    }
  }
};

const setDifficulty = (charts, difficultyName) => {
  const numericDifficultyCharts = charts.filter(
    (it) => it.header.difficulty === "NUMERIC"
  );
  const levels = numericDifficultyCharts.map(
    (it) => `${it.header.name}[${it.header.level}]`
  );
  const levelNames = numericDifficultyCharts.map((it) => it.header.name);

  if (!_.some(charts, (it) => it.header.difficulty === difficultyName)) {
    console.log("-> levels: ".bold + `(${levels.join(", ")})`.cyan);
    const chartName = utils.insistentChoice(
      `Which one is ${difficultyName}?`,
      levelNames
    );

    _.find(
      charts,
      (it) => it.header.name.toLowerCase() === chartName
    ).header.difficulty = difficultyName;

    return true;
  }

  return false;
};
