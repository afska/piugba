const Events = require("./Events");
const _ = require("lodash");

module.exports = class Chart {
  constructor(header, content) {
    this.header = header;
    this.content = content;
  }

  get events() {
    const measures = this._getMeasures();

    let cursor = 0;
    const notes = _.flatMap(measures, (measure, measureId) => {
      const events = measure.split(/\r?\n/);
      const subdivision = 1 / events.length;

      return _.flatMap(events, (line, noteId) => {
        const beat = measureId * UNIT + noteId * subdivision;
        const bpm = this._getBpmByBeat(beat);
        const wholeNoteDuration = (MINUTE / bpm) * UNIT;

        const duration = subdivision * wholeNoteDuration;
        const timestamp = Math.round(this.header.offset + cursor);
        cursor += duration;

        const eventsByType = this._getEventsByType(line);

        // TODO: Meter eventos SET_TEMPO donde corresponda
        // TODO: Meter eventos HOLD_TICK al encontrar un HOLD_END según el tickcount que había en el HOLD_START
        // TODO: Probar DELAYS con level 17

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
        bpm: 123, // TODO: SET BPM EVENTS
      },
    ].concat(notes);
  }

  _getMeasures() {
    return this.content
      .split(",")
      .map((it) => it.trim())
      .filter(_.identity);
  }

  _getEventsByType(line) {
    return _(line)
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
  }

  _getBpmByBeat(beat) {
    return _.findLast(this.header.bpms, (bpm) => beat >= bpm.key).value;
  }
};

const MINUTE = 60000;
const UNIT = 4;
