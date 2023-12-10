const fs = require("fs");
const $path = require("path");
const _ = require("lodash");

const OFFSETS_FILE = "offsets.pofs";
const REGEXP = /(.+)\[(?:s|d|m)(\d\d)\]=([-+]?\d+|delete)/;
const REGEXP_DOUBLE = /(.+)\[(?:d|m)(\d\d)\]=([-+]?\d+|delete)/;
const REGEXP_MULTIPLAYER = /(.+)\[m(\d\d)\]=([-+]?\d+|delete)/;

const getOffsetCorrections = _.memoize(() => {
  let offsetsFile;
  try {
    offsetsFile = fs
      .readFileSync($path.join(GLOBAL_OPTIONS.directory, OFFSETS_FILE))
      .toString();
  } catch (e) {
    return [];
  }

  try {
    return offsetsFile
      .match(new RegExp(REGEXP.source, REGEXP.flags + "g"))
      .map((line) => {
        const isDouble = REGEXP_DOUBLE.test(line);
        const isMultiplayer = REGEXP_MULTIPLAYER.test(line);

        const parts = line.match(REGEXP);
        if (
          _.isEmpty(parts[1]) ||
          _.isEmpty(parts[2]) ||
          _.isEmpty(parts[3]) ||
          !_.isFinite(parseInt(parts[2])) ||
          (!_.isFinite(parseInt(parts[3])) && parts[3] !== "delete")
        )
          throw new Error("invalid_offset_corrections");

        if (parts[3] === "delete")
          return {
            name: parts[1],
            level: parseInt(parts[2]),
            isDouble,
            isMultiplayer,
            isDeleted: true,
          };

        return {
          name: parts[1],
          level: parseInt(parts[2]),
          isDouble,
          isMultiplayer,
          offset: parseInt(parts[3]),
        };
      });
  } catch (e) {
    console.log("  ⚠️  error parsing offset corrections");
    return [];
  }
});

const applyOffsets = (metadata, charts) => {
  const corrections = getOffsetCorrections().filter(
    (it) => it.name.toLowerCase() == metadata.title.toLowerCase()
  );

  corrections.forEach((correction) => {
    const matchingChart = _.find(
      charts,
      (it) =>
        it.header.level == correction.level &&
        it.header.isDouble === correction.isDouble &&
        it.header.isMultiplayer === correction.isMultiplayer
    );

    if (matchingChart != null) apply(matchingChart, correction);
  });

  charts.forEach((chart) => {
    if (chart.header.$originalOffset !== undefined) {
      charts
        .filter(
          (it) =>
            it.header.$originalOffset === undefined &&
            it.header.offset == chart.header.$originalOffset
        )
        .forEach((similarChart) => {
          console.log(
            `  ⚠️  copying offset from ${level(chart)} to ${level(
              similarChart
            )}`
          );
          similarChart.header.$originalOffset = chart.header.offset;
          similarChart.header.offset = chart.header.offset;
        });
    }
  });
};

module.exports = { getOffsetCorrections, applyOffsets };

const apply = (chart, correction) => {
  if (correction.isDeleted) {
    chart.isDeleted = true;
    console.log(`  ⚠️  deleting ${level(chart)} chart`);
    correction.used = true;
    return;
  }

  console.log(`  ⚠️  applying offset ${correction.offset} to ${level(chart)}`);
  chart.header.$originalOffset = chart.header.offset;
  chart.header.offset = chart.header.offset - correction.offset;
  correction.used = true;
};

const level = (chart) => chart.header.levelStr;
