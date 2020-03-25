const _ = require("lodash");

module.exports = class Chart {
  constructor(header, content) {
    this.header = header;
    this.content = content;
  }

  get events() {
    const bpm = parseFloat(_.first(this.header.bpms).value);
    const wholeNoteDuration = (MINUTE / bpm) * UNIT;

    const measures = this.content
      .split(",")
      .map((it) => it.trim())
      .filter(_.identity);

    let cursor = 0;
    return _.flatMap(measures, (measure) => {
      const events = measure.split(/\r?\n/);
      const subdivision = 1 / events.length;
      const duration = subdivision * wholeNoteDuration;

      return events.map((data) => {
        const timestamp = Math.round(this.header.offset + cursor);
        cursor += duration;

        return {
          timestamp,
          type: EVENTS.NOTE,
          arrows: data.split("").map((arrow) => (arrow == 1 ? true : false)),
        };
      });
    }).filter((it) => it.arrows.some(_.identity));
  }
};

const EVENTS = {
  NOTE: 0,
  HOLD_START: 1,
  HOLD_TAIL: 2,
  STOP: 3,
  SET_TEMPO: 4,
  SET_SPEED: 5,
};

const MINUTE = 60000;
const UNIT = 4;
