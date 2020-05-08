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
      _.sortBy([...timingEvents, ...noteEvents], ["timestamp", "type"])
    );
  }

  _getNoteEvents(timingEvents) {
    const measures = this._getMeasures();
    let cursor = 0;

    return _.flatMap(measures, (measure) => {
      // 1 measure = 1 whole note = BEAT_UNIT beats
      const lines = this._getMeasureLines(measure);
      const subdivision = 1 / lines.length;

      return _.flatMap(lines, (line) => {
        const bpm = this._getBpmByTimestamp(cursor, timingEvents);
        const wholeNoteLength = this._getWholeNoteLengthByBpm(bpm);
        const noteDuration = subdivision * wholeNoteLength;
        const timestamp = cursor;
        cursor += noteDuration;
        const eventsByType = this._getEventsByType(line);

        return _(eventsByType)
          .map(({ type, arrows }) => ({
            timestamp,
            type,
            arrows: _.range(0, 5).map((id) => _.includes(arrows, id)),
          }))
          .filter((it) => _.some(it.arrows))
          .reject((it) => this._isInsideWarp(it.timestamp, timingEvents))
          .value();
      });
    });
  }

  _getTimingEvents() {
    const segments = _([
      this.header.warps.map((it) => ({
        type: Events.WARP,
        data: it,
      })),
      [...this.header.stops, ...this.header.delays].map((it) => ({
        type: Events.STOP,
        data: it,
      })),
      this.header.bpms.map((it) => ({ type: Events.SET_TEMPO, data: it })),
      this.header.tickcounts.map((it) => ({
        type: Events.SET_TICKCOUNT,
        data: it,
      })),
      //this.header.scrolls.map((it) => ({ type: Events.STOP_ASYNC, data: it })),
      // TODO: NOW THAT STOPS ARE SYNC IN THE GBA, IMPLEMENT ASYNC STOPS
    ])
      .flatten()
      .sortBy("data.key")
      .value();

    let currentTimestamp = 0;
    let currentBeat = 0;
    let currentBpm = this._getBpmByBeat(0);
    let currentScrollEnabled = true;
    let currentScrollTimestamp = 0;

    return _(segments)
      .map(({ type, data }, i) => {
        const beat = data.key;
        const beatLength = this._getBeatLengthByBpm(currentBpm);
        currentTimestamp += (beat - currentBeat) * beatLength;
        currentBeat = beat;
        currentBpm = this._getBpmByBeat(beat);
        const timestamp = currentTimestamp;

        if (data.value < 0) throw new Error("invalid_negative_timing_segment");

        let length;
        switch (type) {
          case Events.WARP:
            length = this._getRangeDuration(
              currentBeat,
              currentBeat + data.value
            );

            return {
              timestamp,
              type,
              length,
            };
          case Events.STOP:
            length = data.value * SECOND;

            return {
              timestamp,
              type,
              length,
            };
          case Events.STOP_ASYNC:
            const scrollEnabled = data.value > 0;

            if (scrollEnabled && !currentScrollEnabled) {
              length = timestamp - currentScrollTimestamp;
              currentScrollEnabled = true;
              currentScrollTimestamp = timestamp;

              return {
                timestamp,
                type,
                length,
              };
            }

            currentScrollEnabled = scrollEnabled;
            currentScrollTimestamp = timestamp;
            return null;
          case Events.SET_TEMPO:
            return {
              timestamp,
              type,
              bpm: currentBpm,
            };
          case Events.SET_TICKCOUNT:
            return {
              timestamp,
              type,
              tickcount: data.value,
            };
          default:
            throw new Error("unknown_timing_segment");
        }
      })
      .compact()
      .value();
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

  _getMeasureLines(measure) {
    return measure
      .split(/\r?\n/)
      .map((it) => it.replace(/\/\/.*/g, ""))
      .map((it) => it.trim())
      .filter((it) => NOTE_DATA.test(it));
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
    const bpm = _.findLast(this.header.bpms, (bpm) => beat >= bpm.key);
    if (!bpm) return 0;

    return bpm.value;
  }

  _getBpmByTimestamp(timestamp, timingEvents) {
    const event = _.findLast(
      timingEvents,
      (event) => event.type === Events.SET_TEMPO && timestamp >= event.timestamp
    );

    return (event && event.bpm) || 0;
  }

  _getRangeDuration(startBeat, endBeat) {
    let length = 0;

    for (let beat = startBeat; beat < endBeat; beat += FUSE) {
      const bpm = this._getBpmByBeat(beat);
      const beatLength = this._getBeatLengthByBpm(bpm) * FUSE;
      length += beatLength;
    }

    return length;
  }

  _isInsideWarp(timestamp, timingEvents) {
    return _.some(
      timingEvents,
      (event) =>
        event.type === Events.WARP &&
        timestamp >= event.timestamp &&
        timestamp < event.timestamp + event.length
    );
  }
};

const SECOND = 1000;
const MINUTE = 60 * SECOND;
const BEAT_UNIT = 4;
const NOTE_DATA = /^\d\d\d\d\d$/;
const FUSE = 1 / 2 / 2 / 2 / 2;
