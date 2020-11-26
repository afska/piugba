const Channels = require("../../parser/Channels");
const utils = require("../../utils");
const _ = require("lodash");

const KNOWN_CHANNELS = ["ORIGINAL", "KPOP", "WORLD"];
const NON_NUMERIC_LEVELS = ["CRAZY", "HARD", "NORMAL"];

const HEURISTICS = {
  NORMAL: [6, 5, 4, 3, 2, 1, 7, 8],
  HARD: [11, 10, 9, 8, 7, 12, 6],
  CRAZY: [16, 15, 14, 13, 12, 17, 18, 19, 20, 11],
};
const TAGS = ["pro", "new", "ucs", "hidden", "train", "sp", "quest", "another"];

module.exports = (metadata, charts, content, filePath) => {
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
    }
  }

  charts.forEach((it) => {
    if (
      GLOBAL_OPTIONS.mode === "manual" ||
      it.header.isDouble ||
      GLOBAL_OPTIONS.arcade
    )
      it.header.mode = "NUMERIC";
  });

  if (!GLOBAL_OPTIONS.arcade) {
    const singleCharts = charts.filter((it) => !it.header.isDouble);
    NON_NUMERIC_LEVELS.forEach((difficulty) => {
      if (
        !hasDifficulty(singleCharts, difficulty) &&
        GLOBAL_OPTIONS.mode === "auto"
      )
        autoSetDifficulty(singleCharts, difficulty);
      else setDifficulty(singleCharts, difficulty);
    });
    checkLevelOrder(singleCharts);
  }

  return {
    metadata,
    charts: charts.map((it) => ({ header: it.header, events: it.events })),
    getChartByDifficulty(difficulty) {
      return getChartByDifficulty(this.charts, difficulty);
    },
  };
};

const autoSetDifficulty = (charts, difficultyName) => {
  const numericDifficultyCharts = charts.filter(
    (it) => it.header.difficulty === "NUMERIC"
  );
  const sortedNumericDifficultyCharts = _.orderBy(
    numericDifficultyCharts,
    (it) => it.header.level
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
    chart = _.last(sortedNumericDifficultyCharts);
  if (!chart && difficultyName === "HARD") {
    const crazyChart = _.find(charts, (it) => it.header.difficulty === "CRAZY");
    chart = _.findLast(
      sortedNumericDifficultyCharts,
      (it) => it.header.level < crazyChart.header.level
    );
  }
  if (!chart && difficultyName === "NORMAL")
    chart = _.first(sortedNumericDifficultyCharts);

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

const checkLevelOrder = (charts) => {
  const normal = getChartByDifficulty(charts, "NORMAL");
  const hard = getChartByDifficulty(charts, "HARD");
  const crazy = getChartByDifficulty(charts, "CRAZY");
  const level = (chart) => chart.header.level;

  if (
    level(normal) > level(hard) ||
    level(normal) > level(crazy) ||
    level(hard) > level(crazy)
  ) {
    console.error(
      `  ⚠️  sorting difficulties... (${level(normal)}, ${level(hard)}, ${level(
        crazy
      )})`.yellow
    );

    const charts = _.orderBy([normal, hard, crazy], level);
    charts[0].header.difficulty = "NORMAL";
    charts[1].header.difficulty = "HARD";
    charts[2].header.difficulty = "CRAZY";
  }
};

const hasDifficulty = (charts, difficultyName) => {
  return _.some(charts, (it) => it.header.difficulty === difficultyName);
};

const getChartByDifficulty = (charts, difficulty) => {
  return _.find(charts, (chart) => chart.header.difficulty === difficulty);
};
