const fs = require("fs");
const $path = require("path");
const _ = require("lodash");

const OFFSETS_FILE = "offsets.pofs";
const REGEXP = /([\w ]+)\[s(\d\d)\]=([-+]?\d+)/;

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
        const parts = line.match(REGEXP);
        if (
          _.isEmpty(parts[1]) ||
          _.isEmpty(parts[2]) ||
          _.isEmpty(parts[3]) ||
          !_.isFinite(parseInt(parts[2])) ||
          !_.isFinite(parseInt(parts[3]))
        )
          throw new Error("invalid_offset_corrections");

        return {
          name: parts[1],
          level: parseInt(parts[2]),
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
      (it) => it.header.level == correction.level && !it.header.isDouble
    );

    if (matchingChart !== null) apply(matchingChart, correction);
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
  console.log(`  ⚠️  applying offset ${correction.offset} to ${level(chart)}`);
  chart.header.$originalOffset = chart.header.offset;
  chart.header.offset = chart.header.offset - correction.offset;
  correction.used = true;
};

const level = (chart) =>
  `${chart.header.isDouble ? "d" : "s"}${chart.header.level}`;
