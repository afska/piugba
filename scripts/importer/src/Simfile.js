const _ = require("lodash");

const PROPERTY = (name) => new RegExp(`^#${name}:(.+);$`);
const REGEXPS = {
  metadata: {
    title: PROPERTY("TITLE"),
    artist: PROPERTY("ARTIST"),
    genre: PROPERTY("GENRE"),
    sampleStart: PROPERTY("SAMPLESTART"),
    sampleLength: PROPERTY("SAMPLELENGTH"),
  },
  chart: {
    start: /\/\/-+pump-single - (.+)-+\r?\n((.|(\r?\n))*?)#NOTES:/g,
    name: PROPERTY("DESCRIPTION"),
  },
};

// StepMania 5 format (*.ssc)

module.exports = class Simfile {
  constructor(content) {
    this.content = content;
  }

  get metadata() {
    const lines = this._getLinesOf(this.content);

    return _.mapValues(REGEXPS.metadata, (regexp) =>
      this._getSingleMatch(lines, regexp)
    );
  }

  get charts() {
    return this.content.match(REGEXPS.chart.start).map((rawChart) => {
      const lines = this._getLinesOf(rawChart);
      return this._getSingleMatch(lines, REGEXPS.chart.name);
    });
  }

  _getSingleMatch(lines, regexp) {
    const line = _.find(lines, (line) => regexp.test(line));
    const match = line && line.match(regexp);
    return (match && match[1]) || null;
  }

  _getLinesOf(content) {
    return content.split("\n").map((it) => it.trim());
  }
};
