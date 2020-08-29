const Channels = require("../../parser/Channels");
const utils = require("../../utils");
const fs = require("fs");
const _ = require("lodash");

const KNOWN_CHANNELS = ["ORIGINAL", "KPOP", "WORLD", "BOSS"];
const NON_NUMERIC_LEVELS = ["CRAZY", "HARD", "NORMAL"];
const PROP_REGEXP = (name) => `#${name}:((.|(\r|\n))*?);`;
const GLOBAL_PROPERTY = (name) => new RegExp(PROP_REGEXP(name), "g");
const PROPERTY = (name) => new RegExp(PROP_REGEXP(name));
const CHANNEL_PROP = "SONGCATEGORY";
const DIFFICULTY_PROP = "DIFFICULTY";

const HEURISTICS = {
  NORMAL: [6, 5, 4, 3, 2, 1, 7],
  HARD: [11, 10, 9, 8, 7, 12],
  CRAZY: [16, 15, 14, 13, 12, 17, 18, 19, 20],
};
const TAGS = ["new", "ucs", "hidden", "sp", "quest", "another"];

module.exports = (metadata, charts, content, filePath) => {
  let isDirty = false;

  if (metadata.channel === "UNKNOWN") {
    if (GLOBAL_OPTIONS.mode === "auto") metadata.channel = "ORIGINAL";
    else {
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
  }

  if (GLOBAL_OPTIONS.mode === "manual")
    charts.forEach((it) => {
      it.header.mode = "NUMERIC";
    });

  NON_NUMERIC_LEVELS.forEach((difficulty) => {
    if (!hasDifficulty(charts, difficulty) && GLOBAL_OPTIONS.mode === "auto")
      autoSetDifficulty(charts, difficulty);
    else {
      if (setDifficulty(charts, difficulty)) isDirty = true;
    }
  });

  if (isDirty) writeIfNeeded(metadata, charts, content, filePath);

  return {
    metadata,
    charts: GLOBAL_OPTIONS.all
      ? charts
      : charts.filter((it) =>
          _.includes(NON_NUMERIC_LEVELS, it.header.difficulty)
        ),
  };
};

const writeIfNeeded = (metadata, charts, content, filePath) => {
  const writeChanges = utils.insistentChoice(
    "Write simfile? (y/n)",
    ["y", "n"],
    "red"
  );
  if (writeChanges === "y") {
    let newContent = content;

    newContent = newContent.replace(
      GLOBAL_PROPERTY(CHANNEL_PROP),
      `#${CHANNEL_PROP}:${metadata.channel};`
    );

    charts.forEach(({ header }) => {
      newContent = utils.replaceRange(
        newContent,
        PROPERTY(DIFFICULTY_PROP),
        `#${DIFFICULTY_PROP}:${header.difficulty};`,
        header.startIndex
      );
    });

    fs.writeFileSync(filePath, newContent);
  }
};

const autoSetDifficulty = (charts, difficultyName) => {
  const numericDifficultyCharts = charts.filter(
    (it) => it.header.difficulty === "NUMERIC"
  );

  let chart = null;
  for (let level of HEURISTICS[difficultyName]) {
    const candidate = _.find(
      numericDifficultyCharts,
      (it) => it.header.difficulty === "NUMERIC" && it.header.level === level
    );

    if (candidate) chart = getBestChartBetween(candidate, chart);
  }

  if (!chart && difficultyName === "CRAZY")
    chart = _.last(numericDifficultyCharts);
  if (!chart && difficultyName === "NORMAL")
    chart = _.first(numericDifficultyCharts);

  if (!chart)
    throw new Error(
      `cannot_autoset_difficulty: ${difficultyName} - (${charts
        .map((it) => it.header.level)
        .join("|")})`
    );
  chart.header.difficulty = difficultyName;
};

const getBestChartBetween = (candidate, currentChart) => {
  if (currentChart === null) return candidate;

  return getChartAffinityLevel(candidate) < getChartAffinityLevel(currentChart)
    ? candidate
    : currentChart;
};

const getChartAffinityLevel = (chart) => {
  const name = chart.header.name.toLowerCase();
  let affinity = -1;

  for (let i = 0; i < TAGS.length; i++) {
    const tag = TAGS[i];

    if (_.includes(name, tag)) {
      affinity = i;
      break;
    }
  }

  return affinity;
};

const setDifficulty = (charts, difficultyName) => {
  const createId = (chart) => `${chart.header.name}/${chart.header.level}`;
  const numericDifficultyCharts = charts.filter(
    (it) => it.header.difficulty === "NUMERIC"
  );
  const levels = numericDifficultyCharts.map(createId);

  if (!hasDifficulty(charts, difficultyName)) {
    console.log("-> levels: ".bold + `(${levels.join(", ")})`.cyan);
    const chartName = utils.insistentChoice(
      `Which one is ${difficultyName}?`,
      levels
    );

    _.find(
      charts,
      (it) => createId(it).toLowerCase() === chartName
    ).header.difficulty = difficultyName;

    return true;
  }

  return false;
};

const hasDifficulty = (charts, difficultyName) => {
  return _.some(charts, (it) => it.header.difficulty === difficultyName);
};
