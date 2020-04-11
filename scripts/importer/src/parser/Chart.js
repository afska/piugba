const Events = require("./Events");
const _ = require("lodash");

module.exports = class Chart {
  constructor(header, content) {
    this.header = header;
    this.content = content;
  }

  get events() {
    const noteEvents = this._getNoteEvents();
    const tempoEvents = this._getTempoEvents(); // TODO: Calcular tempo & delay events todos juntos, el delay corre los tempo events también
    const delayEvents = this._getDelayEvents();

    // if (this.header.level == 17) {
    //   console.log(delayEvents);
    //   process.exit(1);
    // } // TODO: REMOVE

    return this._applyOffset(
      this._applyDelays(
        _.sortBy([...tempoEvents, ...delayEvents, ...noteEvents], "timestamp")
      )
    );
  }

  _getNoteEvents() {
    const measures = this._getMeasures();
    let cursor = 0;

    return _.flatMap(measures, (measure, measureId) => {
      // 1 measure = 1 whole note = BEAT_UNIT beats
      const events = measure.split(/\r?\n/);
      const subdivision = 1 / events.length;

      return _.flatMap(events, (line, noteId) => {
        // TODO: A este punto, ver el delay total para aplicarlo al offset, this._getBpmByBeat ya no es confiable. Habría que darle el delayOffset, o directamente que sea ByTimestamp
        const beat = measureId * BEAT_UNIT + noteId * subdivision;
        const bpm = this._getBpmByBeat(beat);
        const wholeNoteLength = this._getWholeNoteLengthByBpm(bpm);
        const noteDuration = subdivision * wholeNoteLength;
        const timestamp = cursor;

        cursor += noteDuration;
        const eventsByType = this._getEventsByType(line);

        // TODO: Meter eventos HOLD_TICK al encontrar un HOLD_END según el tickcount que había en el HOLD_START
        // TODO: Probar DELAYS con level 17

        return eventsByType.map(({ type, arrows }) => ({
          timestamp,
          type,
          arrows: _.range(0, 5).map((id) => arrows.includes(id)),
        }));
      });
    });
  }

  _getTempoEvents() {
    return this._mapBeatDictionary(this.header.bpms, (timestamp, { bpm }) => ({
      timestamp,
      type: Events.SET_TEMPO,
      bpm,
    }));
  }

  _getDelayEvents() {
    return this._mapBeatDictionary(
      this.header.delays,
      (timestamp, { value, beatLength }) => ({
        timestamp,
        type: Events.STOP,
        length: Math.round(value * beatLength), // TODO: or seconds?
      })
    );
  }

  _applyDelays(events) {
    let offset = 0;

    return events.map((event) => {
      switch (event.type) {
        case Events.STOP:
          offset += event.length;
          return event;
        case Events.NOTE:
        case Events.HOLD_START:
        case Events.HOLD_END:
          return { ...event, timestamp: offset + event.timestamp };
        default:
          return event;
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

  _mapBeatDictionary(dictionary, transform) {
    let currentTimestamp = 0;
    let currentBeat = 0;
    let currentBpm = this._getBpmByBeat(0);

    return dictionary.map((it, i) => {
      const beat = it.key;
      const beatLength = this._getBeatLengthByBpm(currentBpm);
      currentTimestamp += (beat - currentBeat) * beatLength;
      currentBeat = beat;
      currentBpm = this._getBpmByBeat(beat);

      return transform(currentTimestamp, {
        beat,
        value: it.value,
        bpm: currentBpm,
        beatLength,
      });
    });
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
};

const SECOND = 1000;
const MINUTE = 60 * SECOND;
const BEAT_UNIT = 4;
