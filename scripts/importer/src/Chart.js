const _ = require("lodash");

const MINUTE = 60000;
const UNIT = 4;

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
        const timestamp = cursor;
        cursor += duration;

        return {
          timestamp,
          type: EVENTS.NOTE,
          arrows: data.split("").map((arrow) => (arrow == 1 ? true : false)),
        };
      });
    });
  }
};

// TODO: Reject no-op events

const EVENTS = {
  NOTE: 0,
  HOLD_START: 1,
  HOLD_TAIL: 2,
  HOLD_END: 3,
  STOP: 4,
  SET_SPEED: 5,
};
