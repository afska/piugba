const fs = require("fs");
const $path = require("path");
const _ = require("lodash");

const OFFSETS_FILE = "offsets.pofs";
const REGEXP = /(.+)\[(?:s|d|m)(\d?\d)\](?:\[(\d?\d)\])?=([-+]?\d+|delete)/;
const REGEXP_DOUBLE = /(.+)\[(?:d|m)(\d?\d)\](?:\[(\d?\d)\])?=([-+]?\d+|delete)/;
const REGEXP_MULTIPLAYER = /(.+)\[m(\d?\d)\](?:\[(\d?\d)\])?=([-+]?\d+|delete)/;
const TITLE_LEN = 30;

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
        const name = parts[1];
        const level = parts[2];
        const subindex = parts[3];
        const offset = parts[4];
        const parsedSubindex = _.isFinite(parseInt(subindex))
          ? parseInt(subindex)
          : 0;
        if (
          _.isEmpty(name) ||
          _.isEmpty(level) ||
          _.isEmpty(offset) ||
          !_.isFinite(parseInt(level)) ||
          (!_.isFinite(parseInt(offset)) && offset !== "delete")
        )
          throw new Error("invalid_offset_corrections");

        if (offset === "delete")
          return {
            name,
            level: parseInt(level),
            subindex: parsedSubindex,
            isDouble,
            isMultiplayer,
            isDeleted: true,
          };

        return {
          name,
          level: parseInt(level),
          subindex: parsedSubindex,
          isDouble,
          isMultiplayer,
          offset: parseInt(offset),
        };
      });
  } catch (e) {
    console.log("  ⚠️  error parsing offset corrections");
    return [];
  }
});

const applyOffsets = (metadata, charts) => {
  const corrections = getOffsetCorrections().filter(
    (it) =>
      it.name.toLowerCase().substring(0, TITLE_LEN) ==
      metadata.title.toLowerCase().substring(0, TITLE_LEN)
  );

  corrections.forEach((correction) => {
    const matchingChart = _.filter(
      charts,
      (it) =>
        it.header.level == correction.level &&
        it.header.isDouble === correction.isDouble &&
        it.header.isMultiplayer === correction.isMultiplayer
    )[correction.subindex];

    if (matchingChart != null) apply(matchingChart, correction);
  });

  charts.forEach((chart) => {
    if (
      chart.header.$originalOffset !== undefined &&
      chart.header.$originalOffset !== chart.header.offset
    ) {
      charts
        .filter(
          (it) =>
            !it.isDeleted &&
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
    console.log(`  ⚠️  deleting ${level(chart)}[${correction.subindex}] chart`);
    correction.used = true;
    return;
  }

  console.log(
    `  ⚠️  applying offset ${correction.offset} to ${level(chart)}[${
      correction.subindex
    }]`
  );
  chart.header.$originalOffset = chart.header.offset;
  chart.header.offset = chart.header.offset - correction.offset; // [!] offsets in PIUS are negative
  correction.used = true;
};

const level = (chart) => chart.header.levelStr;
