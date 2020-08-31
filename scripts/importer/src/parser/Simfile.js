const Chart = require("./Chart");
const Channels = require("./Channels");
const DifficultyLevels = require("./DifficultyLevels");
const Mods = require("./Mods");
const utils = require("../utils");
const _ = require("lodash");

// StepMania 5 format (*.ssc)
// (Charts must end with the "NOTES" tag)

module.exports = class Simfile {
  constructor(content) {
    this.content = content;
  }

  get metadata() {
    const title = this._getSingleMatch(REGEXPS.metadata.title);
    const subtitle = this._getSingleMatch(REGEXPS.metadata.subtitle);
    const custom = this._getSingleMatch(REGEXPS.metadata.custom);

    const config = {
      ..._.mapValues(Mods, (v, k) =>
        _.isFinite(v[custom[k]]) ? custom[k] : _.findKey(v, (it) => it === 0)
      ),
      APPLY_TO:
        _.isString(custom.APPLY_TO) && custom.APPLY_TO.length === 3
          ? custom.APPLY_TO.split("").map((c) => c === "1")
          : [false, false, false],
      IS_BOSS: custom.IS_BOSS === "TRUE",
      MESSAGE: custom.MESSAGE || "",
    };

    return {
      title: subtitle || title,
      artist: this._getSingleMatch(REGEXPS.metadata.artist),
      channel:
        this._getSingleMatchFromEnum(REGEXPS.metadata.channel, Channels) ||
        "UNKNOWN",
      lastMillisecond: this._toMilliseconds(
        this._getSingleMatch(REGEXPS.metadata.lastSecondHint) || 999999
      ),
      sampleStart: this._toMilliseconds(
        this._getSingleMatch(REGEXPS.metadata.sampleStart)
      ),
      sampleLength: this._toMilliseconds(
        this._getSingleMatch(REGEXPS.metadata.sampleLength)
      ),
      config,
    };
  }

  get charts() {
    const charts = _.compact(
      this.content.match(REGEXPS.chart.start).map((rawChart) => {
        const startIndex = this.content.indexOf(rawChart);

        const name =
          this._getSingleMatch(REGEXPS.chart.name, rawChart, true) || "";
        const difficulty =
          this._getSingleMatchFromEnum(
            REGEXPS.chart.difficulty,
            DifficultyLevels,
            rawChart
          ) || "NUMERIC";
        const level = utils.restrictTo(
          this._getSingleMatch(REGEXPS.chart.level, rawChart, true),
          0,
          99
        );
        if (!_.isFinite(level)) throw new Error("no_level_info");

        let chartOffset = this._getSingleMatch(REGEXPS.chart.offset, rawChart);
        if (!_.isFinite(chartOffset)) chartOffset = 0;
        const offset = -chartOffset * SECOND;

        const bpms = this._getSingleMatch(REGEXPS.chart.bpms, rawChart);
        if (_.isEmpty(bpms)) {
          console.log(name, difficulty, bpms);
          process.exit(1);
        }
        if (_.isEmpty(bpms)) throw new Error("no_bpm_info");

        const speeds = this._getSingleMatch(REGEXPS.chart.speeds, rawChart);
        const tickcounts = this._getSingleMatch(
          REGEXPS.chart.tickcounts,
          rawChart
        );
        const fakes = this._getSingleMatch(REGEXPS.chart.fakes, rawChart);
        const stops = this._getSingleMatch(REGEXPS.chart.stops, rawChart);
        const delays = this._getSingleMatch(REGEXPS.chart.delays, rawChart);
        const scrolls = this._getSingleMatch(REGEXPS.chart.scrolls, rawChart);
        const warps = this._getSingleMatch(REGEXPS.chart.warps, rawChart);
        const header = {
          startIndex,
          name,
          difficulty,
          level,
          offset,
          bpms,
          speeds,
          tickcounts,
          fakes,
          stops,
          delays,
          scrolls,
          warps,
        };

        const notesStart = startIndex + rawChart.length;
        const rawNotes = this._getSingleMatch(
          REGEXPS.limit,
          this.content.substring(notesStart)
        );

        try {
          const events = new Chart(this.metadata, header, rawNotes).events;
          return { header, events };
        } catch (e) {
          console.error(`  ⚠️  level-${level} error: ${e.message}`.yellow);
          return null;
        }
      })
    );

    return _.sortBy(charts, "header.level");
  }

  _getSingleMatch(regexp, content = this.content, isChartExclusive = false) {
    const exp = regexp.exp || regexp;
    const parse = regexp.parse || _.identity;

    const match = content && content.match(exp);
    const rawData = (match && match[1]) || null;
    const parsedData = parse(rawData);
    const finalData = _.isString(parsedData)
      ? this._toAsciiOnly(parsedData)
      : parsedData;

    return content !== this.content && rawData === null && !isChartExclusive
      ? this._getSingleMatch(regexp)
      : finalData;
  }

  _getSingleMatchFromEnum(regexp, options, content = this.content) {
    const match = this._getSingleMatch(regexp, content);
    return _(options).keys(options).includes(match) ? match : null;
  }

  _toAsciiOnly(string) {
    return string.replace(/[^\x00-\x7F]+/g, "");
  }

  _toMilliseconds(float) {
    return Math.round(float * 1000);
  }
};

const PROPERTY = (name) => new RegExp(`#${name}:((.|(\r|\n))*?);`);

const PROPERTY_INT = (name) => ({
  exp: PROPERTY(name),
  parse: (content) => parseInt(content),
});

const PROPERTY_FLOAT = (name) => ({
  exp: PROPERTY(name),
  parse: (content) => parseFloat(content),
});

const DICTIONARY = (name, elements = 2) => ({
  exp: PROPERTY(name),
  parse: (content) =>
    _(content)
      .split(",")
      .map((it) => it.trim().split("="))
      .filter((it) => it.length === elements)
      .map(([key, value, param1, param2]) => ({
        key: parseFloat(key),
        value: parseFloat(value),
        param1: parseFloat(param1),
        param2: parseFloat(param2),
      }))
      .sortBy("key")
      .value(),
});

const OBJECT = (name, elements = 2) => ({
  exp: PROPERTY(name),
  parse: (content) =>
    _(content)
      .split(",,,")
      .map((it) => it.trim().split("="))
      .filter((it) => it.length === elements)
      .map(([key, value]) => ({
        key,
        value: _.trim(value),
      }))
      .keyBy("key")
      .mapValues("value")
      .value(),
});

const REGEXPS = {
  limit: /((.|(\r|\n))*?);/,
  metadata: {
    title: PROPERTY("TITLE"),
    subtitle: PROPERTY("SUBTITLE"),
    artist: PROPERTY("ARTIST"),
    channel: PROPERTY("SONGCATEGORY"),
    lastSecondHint: PROPERTY_FLOAT("LASTSECONDHINT"),
    sampleStart: PROPERTY_FLOAT("SAMPLESTART"),
    sampleLength: PROPERTY_FLOAT("SAMPLELENGTH"),
    custom: OBJECT("PIUGBA"),
  },
  chart: {
    start: /\/\/-+pump-single - (.+)-+\r?\n((.|(\r?\n))*?)#NOTES:/g,
    name: PROPERTY("DESCRIPTION"),
    difficulty: PROPERTY("DIFFICULTY"),
    level: PROPERTY_INT("METER"),
    offset: PROPERTY_FLOAT("OFFSET"),
    bpms: DICTIONARY("BPMS"),
    speeds: DICTIONARY("SPEEDS", 4),
    tickcounts: DICTIONARY("TICKCOUNTS"),
    fakes: DICTIONARY("FAKES"),
    stops: DICTIONARY("STOPS"),
    delays: DICTIONARY("DELAYS"),
    scrolls: DICTIONARY("SCROLLS"),
    warps: DICTIONARY("WARPS"),
  },
};

const SECOND = 1000;
