const _ = require("lodash");

const REGEXPS = {
  METADATA: {
    title: /^#TITLE:(.+);$/,
    artist: /^#ARTIST:(.+);$/,
    genre: /^#GENRE:(.+);$/,
    sampleStart: /^#SAMPLESTART:(.+);$/,
    sampleLength: /^#SAMPLELENGTH:(.+);$/,
  },
};

// StepMania 5 format (*.ssc)

module.exports = class SongChart {
  constructor(content) {
    this.content = content;
  }

  get metadata() {
    const lines = this.lines;

    return _.mapValues(REGEXPS.METADATA, (regex) => {
      const line = _.find(lines, (line) => regex.test(line));
      const match = line && line.match(regex);
      return (match && match[1]) || null;
    });
  }

  get lines() {
    return this.content.split("\n").map((it) => it.trim());
  }
};
