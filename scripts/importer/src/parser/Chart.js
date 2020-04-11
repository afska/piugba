const Events = require("./Events");
const _ = require("lodash");

module.exports = class Chart {
  constructor(header, content) {
    this.header = header;
    this.content = content;
  }

  get events() {
    const timingEvents = this._getTimingEvents();
    const noteEvents = this._getNoteEvents(timingEvents);

    return this._applyOffset(
      _.sortBy([...timingEvents, ...noteEvents], "timestamp")
    );
  }

  _getNoteEvents(timingEvents) {
    const measures = this._getMeasures();
    let cursor = 0;

    return _.flatMap(measures, (measure) => {
      // 1 measure = 1 whole note = BEAT_UNIT beats
      const events = measure.split(/\r?\n/);
      const subdivision = 1 / events.length;

      return _.flatMap(events, (line) => {
        const stop = this._getStopByTimestamp(cursor, timingEvents);
        if (stop && !stop.handled) {
          stop.handled = true;
          cursor += stop.length;
        }
        const bpm = this._getBpmByTimestamp(cursor, timingEvents);
        const wholeNoteLength = this._getWholeNoteLengthByBpm(bpm);
        const noteDuration = subdivision * wholeNoteLength;
        const timestamp = cursor;
        cursor += noteDuration;
        const eventsByType = this._getEventsByType(line);

        // TODO: Meter eventos HOLD_TICK al encontrar un HOLD_END según el tickcount que había en el HOLD_START

        return eventsByType.map(({ type, arrows }) => ({
          timestamp,
          type,
          arrows: _.range(0, 5).map((id) => arrows.includes(id)),
        }));
      });
    });
  }

  _getTimingEvents() {
    const segments = _([
      this.header.delays.map((it) => ({ type: Events.STOP, data: it })),
      this.header.bpms.map((it) => ({ type: Events.SET_TEMPO, data: it })),
    ])
      .flatten()
      .sortBy("data.key")
      .value();

    let currentTimestamp = 0;
    let currentBeat = 0;
    let currentBpm = this._getBpmByBeat(0);

    return segments.map(({ type, data }, i) => {
      const beat = data.key;
      const beatLength = this._getBeatLengthByBpm(currentBpm);
      currentTimestamp += (beat - currentBeat) * beatLength;
      currentBeat = beat;
      currentBpm = this._getBpmByBeat(beat);
      const timestamp = currentTimestamp;

      switch (type) {
        case Events.STOP:
          const length = data.value * SECOND; // beatLength; // TODO: or seconds?
          currentTimestamp += length;

          return {
            timestamp,
            type,
            length: Math.round(length),
          };
        case Events.SET_TEMPO:
          return {
            timestamp,
            type,
            bpm: currentBpm,
          };
        default:
          throw new Error("unknown_timing_segment");
      }
    });
  }

  _applyOffset(events) {
    return events.map((it) => ({
      ...it,
      timestamp: Math.round(this.header.offset + it.timestamp),
    }));
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

  _getWholeNoteLengthByBpm(bpm) {
    return this._getBeatLengthByBpm(bpm) * BEAT_UNIT;
  }

  _getBeatLengthByBpm(bpm) {
    if (bpm === 0) return 0;
    return MINUTE / bpm;
  }

  _getBpmByBeat(beat) {
    return _.findLast(this.header.bpms, (bpm) => beat >= bpm.key).value;
  }

  _getBpmByTimestamp(timestamp, timingEvents) {
    const event = _.findLast(
      timingEvents,
      (event) => event.type === Events.SET_TEMPO && timestamp >= event.timestamp
    );

    return (event && event.bpm) || 0;
  }

  _getStopByTimestamp(timestamp, timingEvents) {
    const event = _.findLast(
      timingEvents,
      (event) => event.type === Events.STOP && timestamp >= event.timestamp
    );

    return event && timestamp < event.timestamp + event.length ? event : null;
  }
};

const SECOND = 1000;
const MINUTE = 60 * SECOND;
const BEAT_UNIT = 4;
