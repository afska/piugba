const Events = require("./Events");
const _ = require("lodash");

module.exports = class Chart {
  static get MAX_SECONDS() {
    return MAX_TIMESTAMP / 1000;
  }

  constructor(metadata, header, content) {
    this.metadata = metadata;
    this.header = header;
    this.content = content;

    this._calculateLastTimestamp();
    this._validate();
  }

  get events() {
    const timingEvents = this._getTimingEvents();
    const noteEvents = this._getNoteEvents(timingEvents);

    return this._applyOffset(
      this._applyFakes(
        this._applyAsyncStops(this._sort([...timingEvents, ...noteEvents]))
      )
    );
  }

  _getNoteEvents(timingEvents) {
    const measures = this._getMeasures();
    let cursor = 0;

    return _.flatMap(measures, (measure, measureIndex) => {
      // 1 measure = 1 whole note = BEAT_UNIT beats
      const lines = this._getMeasureLines(measure);
      const subdivision = 1 / lines.length;

      return _.flatMap(lines, (line, noteIndex) => {
        const beat = (measureIndex + noteIndex * subdivision) * BEAT_UNIT;
        const bpm = this._getBpmByBeat(beat, timingEvents);
        const wholeNoteLength = this._getWholeNoteLengthByBpm(bpm);
        const noteDuration = subdivision * wholeNoteLength;
        const timestamp = cursor;
        cursor += noteDuration;
        const eventsByType = this._getEventsByType(line);

        return _(eventsByType)
          .map(({ type, arrows }) => {
            const activeArrows = _.range(
              0,
              this.header.isDouble ? 10 : 5
            ).map((id) => _.includes(arrows, id));

            const arrowCount = _.sumBy(activeArrows);
            const isJump = arrowCount > 1;
            const isHold = type === Events.HOLD_START;
            const complexity =
              (type === Events.NOTE || isHold) &&
              this.lastTimestamp < MAX_TIMESTAMP
                ? ((1 - subdivision) *
                    Math.log(bpm) *
                    (isJump ? Math.log2(2 + arrowCount) : 1) *
                    (isHold ? 1.3 : 1)) /
                  (this.lastTimestamp / SECOND)
                : null;

            return {
              timestamp,
              type: type === Events.FAKE_TAP ? Events.NOTE : type,
              playerId: 0,
              arrows: activeArrows.slice(0, 5),
              arrows2: this.header.isDouble ? activeArrows.slice(5, 10) : null,
              complexity,
              isFake: type === Events.FAKE_TAP,
            };
          })
          .filter((it) => _.some(it.arrows) || _.some(it.arrows2))
          .reject(
            (it) =>
              it.type === Events.NOTE &&
              this._isInsideWarp(it.timestamp, timingEvents)
          )
          .value();
      });
    });
  }

  _getTimingEvents() {
    const segments = _([
      this.header.bpms.map((it) => ({ type: Events.SET_TEMPO, data: it })),
      this.header.speeds.map((it) => ({ type: Events.SET_SPEED, data: it })),
      this.header.tickcounts.map((it) => ({
        type: Events.SET_TICKCOUNT,
        data: it,
      })),
      this.header.fakes.map((it) => ({
        type: Events.SET_FAKE,
        data: it,
      })),
      [...this.header.stops, ...this.header.delays].map((it) => ({
        type: Events.STOP,
        data: it,
      })),
      this.header.scrolls.map((it) => ({ type: Events.STOP_ASYNC, data: it })),
      this.header.warps.map((it) => ({
        type: Events.WARP,
        data: it,
      })),
    ])
      .flatten()
      .sortBy(["data.key", "type"])
      .value();

    let currentTimestamp = 0;
    let currentBeat = 0;
    let currentBpm = this._getBpmByBeat(0);
    let warpStart = -1;
    let scrollFactor = 1;
    let currentScrollEnabled = true;
    let currentScrollTimestamp = 0;

    return _(segments)
      .flatMap(({ type, data }) => {
        const beat = data.key;
        const beatLength = this._getBeatLengthByBpm(currentBpm);
        currentTimestamp += (beat - currentBeat) * beatLength;
        currentBeat = beat;
        currentBpm = this._getBpmByBeat(beat);
        const timestamp = currentTimestamp;

        if (data.value < 0)
          throw new Error(
            "invalid_negative_timing_segment:\n    " + JSON.stringify(data)
          );
        const createWarp = () => {
          const length = timestamp - warpStart;
          if (length === 0) return null;

          return {
            timestamp: warpStart,
            type: Events.WARP,
            length: timestamp - warpStart,
          };
        };

        switch (type) {
          case Events.SET_TEMPO: {
            if (data.value > FAST_BPM_WARP) {
              if (warpStart === -1) warpStart = timestamp;
              return null;
            }

            const bpmChange = {
              timestamp,
              type,
              bpm: currentBpm,
              scrollBpm: currentBpm * scrollFactor,
              scrollChangeFrames: 0,
            };

            if (warpStart > -1) {
              const warp = createWarp();
              warpStart = -1;
              return [warp, bpmChange];
            } else {
              return bpmChange;
            }
          }
          case Events.SET_SPEED: {
            scrollFactor = data.value;
            const scrollChangeFrames =
              (data.param2 === 0
                ? data.param1 * beatLength
                : data.param1 * SECOND) / FRAME_MS;

            return {
              timestamp,
              type: Events.SET_TEMPO,
              bpm: currentBpm,
              scrollBpm: currentBpm * scrollFactor,
              scrollChangeFrames,
            };
          }
          case Events.SET_TICKCOUNT: {
            return {
              timestamp,
              type,
              tickcount: data.value,
            };
          }
          case Events.SET_FAKE: {
            return {
              timestamp,
              type,
              endTime: timestamp + data.value * beatLength,
            };
          }
          case Events.STOP: {
            const length = data.value * SECOND;
            const stop = { timestamp, type, length, judgeable: false };

            if (warpStart > -1) {
              const warp = createWarp();
              warpStart = timestamp;

              return [warp, stop];
            } else {
              return stop;
            }
          }
          case Events.STOP_ASYNC: {
            const scrollEnabled = data.value > 0;

            if (!currentScrollEnabled && scrollEnabled) {
              const length = timestamp - currentScrollTimestamp;
              currentScrollEnabled = true;

              return {
                timestamp: currentScrollTimestamp,
                type,
                length,
                judgeable: true,
              };
            }

            if (currentScrollEnabled && !scrollEnabled) {
              currentScrollEnabled = false;
              currentScrollTimestamp = timestamp;
            }

            return null;
          }
          case Events.WARP: {
            const length = this._getRangeDuration(
              currentBeat,
              currentBeat + data.value
            );

            return {
              timestamp,
              type,
              length,
            };
          }
          default:
            throw new Error("unknown_timing_segment: " + type);
        }
      })
      .compact()
      .value();
  }

  _applyOffset(events) {
    return events.map((it) => ({
      ...it,
      timestamp: this.header.offset + it.timestamp,
    }));
  }

  _applyAsyncStops(events) {
    let stoppedTime = 0;

    return this._sort(
      _(events)
        .map((it) => {
          if (it.type === Events.STOP_ASYNC) {
            const timestamp = it.timestamp - stoppedTime;
            stoppedTime += it.length;

            return {
              ...it,
              timestamp,
              type: Events.STOP,
            };
          }

          return {
            ...it,
            timestamp: it.timestamp - stoppedTime,
          };
        })
        .compact()
        .value()
    );
  }

  _applyFakes(events) {
    let fakeEndTime = -1;

    return _.flatMap(events, (it) => {
      let event = it;

      if (it.isFake && fakeEndTime == -1) {
        return [
          {
            timestamp: it.timestamp,
            type: Events.SET_FAKE,
            enabled: 1,
          },
          event,
          {
            timestamp: it.timestamp,
            type: Events.SET_FAKE,
            enabled: 0,
          },
        ];
      }

      if (it.type === Events.SET_FAKE) {
        fakeEndTime = it.endTime;

        event = {
          timestamp: it.timestamp,
          type: it.type,
          enabled: 1,
        };
      }

      if (fakeEndTime !== -1 && it.timestamp > fakeEndTime) {
        fakeEndTime = -1;

        return [
          {
            timestamp: it.timestamp,
            type: Events.SET_FAKE,
            enabled: 0,
          },
          event,
        ];
      }

      return event;
    });
  }

  _sort(events) {
    return _.sortBy(events, [(it) => Math.round(it.timestamp), "type"]);
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
      .filter((it) => !_.isEmpty(it))
      .map((it) =>
        it.replace(/{(\w)\|\w\|(\w)\|\w}/g, (__, note, fake) =>
          fake === "1" ? "F" : note
        )
      ) // weird f2 event syntax
      .map((it) => it.replace(/[MK]/g, "0")) // ignored SSC events
      .filter((it) => {
        const isValid = (this.header.isDouble
          ? NOTE_DATA_DOUBLE
          : NOTE_DATA_SINGLE
        ).test(it);

        if (!isValid) throw new Error("invalid_line: " + it);
        return true;
      });
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
    const bpm = _.findLast(this._getFiniteBpms(), (bpm) => beat >= bpm.key);
    if (!bpm) return 0;

    return bpm.value;
  }

  _getRangeDuration(startBeat, endBeat) {
    let length = 0;

    for (let beat = startBeat; beat < endBeat; beat += SEMIFUSE) {
      const bpm = this._getBpmByBeat(beat);
      length += this._getBeatLengthByBpm(bpm) * SEMIFUSE;
    }

    return length;
  }

  _getFiniteBpms() {
    return this.header.bpms.filter((it) => it.value <= FAST_BPM_WARP);
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

  _calculateLastTimestamp() {
    this.lastTimestamp = MAX_TIMESTAMP;

    try {
      this.lastTimestamp =
        this.metadata.lastMillisecond < MAX_TIMESTAMP
          ? this.metadata.lastMillisecond
          : _.last(this.events).timestamp;
    } catch (e) {}
  }

  _validate() {
    const bpms = this._getFiniteBpms();
    let lastBeat = -1;
    for (let bpm of bpms) {
      if (Math.abs(bpm.key - lastBeat) < SEMIFUSE)
        throw new Error("bpm_change_too_fast:\n    " + JSON.stringify(bpm));
      lastBeat = bpm.key;
    }
  }
};

const SECOND = 1000;
const MINUTE = 60 * SECOND;
const FRAME_MS = 17;
const BEAT_UNIT = 4;
const FAST_BPM_WARP = 9999999;
const NOTE_DATA_SINGLE = /^[\dF][\dF][\dF][\dF][\dF]$/;
const NOTE_DATA_DOUBLE = /^[\dF][\dF][\dF][\dF][\dF][\dF][\dF][\dF][\dF][\dF]$/;
const SEMIFUSE = 1 / 2 / 2 / 2 / 2 / 2;
const MAX_TIMESTAMP = 3600000;
