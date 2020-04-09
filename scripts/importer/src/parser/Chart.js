const Events = require("./Events");
const _ = require("lodash");

module.exports = class Chart {
  constructor(header, content) {
    this.header = header;
    this.content = content;
  }

  get events() {
    const bpm = parseFloat(_.first(this.header.bpms).value);
    const wholeNoteDuration = (MINUTE / bpm) * UNIT;

    // if (this.header.bpms.length > 1)
    //   throw new Error("Multiple BPM values are not supported");
    // TODO: Implement multiple bpm

    const measures = this.content
      .split(",")
      .map((it) => it.trim())
      .filter(_.identity);

    let cursor = 0;
    const notes = _.flatMap(measures, (measure) => {
      const events = measure.split(/\r?\n/);
      const subdivision = 1 / events.length;
      const duration = subdivision * wholeNoteDuration;

      return _.flatMap(events, (data) => {
        const timestamp = Math.round(this.header.offset + cursor);
        cursor += duration;

        const eventsByType = _(data)
          .split("")
          .map((eventType, i) => ({ i, eventType }))
          .groupBy("eventType")
          .map((events, eventType) => ({
            type: Events.parse(eventType),
            arrows: events.map((it) => it.i),
          }))
          .values()
          .filter((it) => it.type != null)
          .value();

        return eventsByType.map(({ type, arrows }) => ({
          timestamp,
          type,
          arrows: _.range(0, 5).map((id) => arrows.includes(id)),
        }));
      });
    });

    return [
      {
        timestamp: 0,
        type: Events.SET_TEMPO,
        bpm: bpm,
      },
    ].concat(notes);
  }
};

const MINUTE = 60000;
const UNIT = 4;
