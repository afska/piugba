const Chart = require("./Chart");
const _ = require("lodash");

// StepMania 5 format (*.ssc)
// (Charts must end with the "NOTES" tag)

module.exports = class Simfile {
  constructor(content, title) {
    this.content = content;
    this.title = title;
  }

  get metadata() {
    return {
      title: this.title || this._getSingleMatch(REGEXPS.metadata.title),
      artist: this._getSingleMatch(REGEXPS.metadata.artist),
      genre: this._getSingleMatch(REGEXPS.metadata.genre),
      sampleStart: this._toMilliseconds(
        this._getSingleMatch(REGEXPS.metadata.sampleStart)
      ),
      sampleLength: this._toMilliseconds(
        this._getSingleMatch(REGEXPS.metadata.sampleLength)
      ),
    };
  }

  get charts() {
    return this.content.match(REGEXPS.chart.start).map((rawChart) => {
      const name = this._getSingleMatch(REGEXPS.chart.name, rawChart);
      const level = this._getSingleMatch(REGEXPS.chart.level, rawChart);
      const offset =
        -this._getSingleMatch(REGEXPS.chart.offset, rawChart) * SECOND;
      const bpms = this._getSingleMatch(REGEXPS.chart.bpms, rawChart);
      const stops = this._getSingleMatch(REGEXPS.chart.stops, rawChart);
      const delays = this._getSingleMatch(REGEXPS.chart.delays, rawChart);
      const scrolls = this._getSingleMatch(REGEXPS.chart.scrolls, rawChart);
      const header = { name, level, offset, bpms, stops, delays, scrolls };

      const notesStart = this.content.indexOf(rawChart) + rawChart.length;
      const rawNotes = this._getSingleMatch(
        REGEXPS.limit,
        this.content.substring(notesStart)
      );
      const events = new Chart(header, rawNotes).events;

      return { header, events };
    });
  }

  _getSingleMatch(regexp, content = this.content) {
    const exp = regexp.exp || regexp;
    const parse = regexp.parse || _.identity;

    const match = content && content.match(exp);
    const result = parse((match && match[1]) || null);
    return _.isString(result) ? this._toAsciiOnly(result) : result;
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

const DICTIONARY = (name) => ({
  exp: PROPERTY(name),
  parse: (content) =>
    _(content)
      .split(",")
      .map((it) => it.trim().split("="))
      .filter((it) => it.length == 2)
      .map(([key, value]) => ({
        key: parseFloat(key),
        value: parseFloat(value),
      }))
      .sortBy("key")
      .value(),
});

const REGEXPS = {
  limit: /((.|(\r|\n))*?);/,
  metadata: {
    title: PROPERTY("TITLE"),
    artist: PROPERTY("ARTIST"),
    genre: PROPERTY("GENRE"),
    sampleStart: PROPERTY_FLOAT("SAMPLESTART"),
    sampleLength: PROPERTY_FLOAT("SAMPLELENGTH"),
  },
  chart: {
    start: /\/\/-+pump-single - (.+)-+\r?\n((.|(\r?\n))*?)#NOTES:/g,
    name: PROPERTY("DESCRIPTION"),
    level: PROPERTY_INT("METER"),
    offset: PROPERTY_FLOAT("OFFSET"),
    bpms: DICTIONARY("BPMS"),
    stops: DICTIONARY("STOPS"),
    delays: DICTIONARY("DELAYS"),
    scrolls: DICTIONARY("SCROLLS"),
  },
};

const SECOND = 1000;
