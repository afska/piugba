const Channels = require("../../parser/Channels");
const utils = require("../../utils");
const fs = require("fs");
const _ = require("lodash");

const NON_NUMERIC_LEVELS = ["NORMAL", "HARD", "CRAZY"];
const GLOBAL_PROPERTY = (name) => new RegExp(`#${name}:((.|(\r|\n))*?);`, "g");
const CHANNEL_PROP = "SONGCATEGORY";
const DIFFICULTY_PROP = "DIFFICULTY";

module.exports = (metadata, charts, content, filePath) => {
  let isDirty = false;

  if (
    GLOBAL_OPTIONS.difficulty === "auto" ||
    GLOBAL_OPTIONS.difficulty === "overwrite"
  ) {
    charts.forEach((it) => {
      it.header.difficulty = "NUMERIC";
    });
  }

  NON_NUMERIC_LEVELS.forEach((difficulty) => {
    if (GLOBAL_OPTIONS.difficulty === "auto") {
      autoSetDifficulty(charts, difficulty);
    } else {
      if (setDifficulty(charts, difficulty)) isDirty = true;
    }
  });

  if (isDirty) {
    const writeChanges = utils.insistentChoice(
      "Write simfile? (y/n)",
      ["y", "n"],
      "red"
    );
    if (writeChanges === "y") {
      let newContent = content.replace(
        GLOBAL_PROPERTY(CHANNEL_PROP),
        `#${CHANNEL_PROP}:${metadata.channel};`
      );
      charts.forEach(({ header }) => {
        newContent = utils.replaceRange(
          newContent,
          GLOBAL_PROPERTY(DIFFICULTY_PROP),
          `#${DIFFICULTY_PROP}:${header.difficulty};`,
          header.startIndex
        );
      });

      fs.writeFileSync(filePath, newContent);
    }
  }

  return {
    metadata,
    charts: GLOBAL_OPTIONS.all
      ? charts
      : charts.filter((it) =>
          _.includes(NON_NUMERIC_LEVELS, it.header.difficulty)
        ),
  };
};

const autoSetDifficulty = (charts, difficultyName) => {
  const HEURISTICS = {
    NORMAL: [6, 5, 4, 3, 2, 1],
    HARD: [11, 10, 9, 8, 7],
    CRAZY: [16, 15, 14, 13, 12],
  };
  const INSANE_LEVEL = 17;

  const numericDifficultyCharts = charts.filter(
    (it) => it.header.difficulty === "NUMERIC"
  );

  let chart;
  for (let level of HEURISTICS[difficultyName]) {
    const candidate = _.find(
      numericDifficultyCharts,
      (it) => it.header.difficulty === "NUMERIC" && it.header.level === level
    );

    if (candidate) {
      chart = candidate;
      break;
    }
  }

  if (!chart && difficultyName === "CRAZY")
    chart = _.find(
      numericDifficultyCharts,
      (it) =>
        it.header.difficulty === "NUMERIC" && it.header.level >= INSANE_LEVEL
    );

  if (!chart && GLOBAL_OPTIONS.force) chart = _.sample(numericDifficultyCharts);

  if (!chart)
    throw new Error(
      `cannot_autoset_difficulty: ${difficultyName} - (${charts
        .map((it) => it.header.level)
        .join("|")})`
    );
  chart.header.difficulty = difficultyName;
};

const setDifficulty = (charts, difficultyName) => {
  const createId = (chart) => `${chart.header.name}/${chart.header.level}`;
  const numericDifficultyCharts = charts.filter(
    (it) => it.header.difficulty === "NUMERIC"
  );
  const levels = numericDifficultyCharts.map(createId);

  if (!_.some(charts, (it) => it.header.difficulty === difficultyName)) {
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
