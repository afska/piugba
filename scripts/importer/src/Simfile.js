const _ = require("lodash");

// StepMania 5 format (*.ssc)
// (Charts must end with the "NOTES" tag)

module.exports = class Simfile {
  constructor(content) {
    this.content = content;
  }

  get metadata() {
    return _.mapValues(REGEXPS.metadata, (regexp) =>
      this._getSingleMatch(regexp)
    );
  }

  get charts() {
    return this.content.match(REGEXPS.chart.start).map((rawChart) => {
      const name = this._getSingleMatch(REGEXPS.chart.name, rawChart);
      const level = this._getSingleMatch(REGEXPS.chart.level, rawChart);
      const offset = this._getSingleMatch(REGEXPS.chart.offset, rawChart);
      const bpms = this._getSingleMatch(REGEXPS.chart.bpms, rawChart);

      const notesStart = this.content.indexOf(rawChart) + rawChart.length;
      const rawNotes = this._getSingleMatch(
        REGEXPS.limit,
        this.content.substring(notesStart)
      );

      return { name, level, offset, bpms, rawNotes };
    });
  }

  _getSingleMatch(regexp, content = this.content) {
    const exp = regexp.exp || regexp;
    const parse = regexp.parse || _.identity;

    const match = content && content.match(exp);
    return parse((match && match[1]) || null);
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
    content
      .split(",")
      .map((it) => it.trim().split("="))
      .map(([key, value]) => ({ key, value })),
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
  },
};
