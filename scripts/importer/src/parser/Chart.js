const Events = require("./Events");
const _ = require("lodash");

/** A StepMania SSC chart. */
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

  /**
   * Generates all the events from the chart.
   * Timing events are metadata events (such as BPM changes, Stops, Warps, etc.)
   * Note events are specifically NOTE, HOLD_START, HOLD_END and FAKE_TAP
   */
  get events() {
    const timingEvents = this._getTimingEvents().map((it, i) => ({
      id: 1 + i,
      ...it,
    }));
    const noteEvents = this._getNoteEvents(timingEvents);

    return this._applyOffset(
      this._sortStopWarps(
        this._applyAsyncStopsAndAddHoldLengths(
          this._applyFakes(this._sort([...timingEvents, ...noteEvents]))
        )
      )
    );
  }

  /** Generates events specifically from note data. */
  _getNoteEvents(timingEvents) {
    const measures = this._getMeasures();
    let cursor = 0;
    let currentId = 0;
    const holdArrows = [];

    return _.flatMap(measures, (measure, measureIndex) => {
      // 1 measure = 1 whole note = BEAT_UNIT beats
      const lines = this._getMeasureLines(measure);
      const subdivision = 1 / lines.length;

      return _.flatMap(lines, (line, noteIndex) => {
        const beat = (measureIndex + noteIndex * subdivision) * BEAT_UNIT;
        const bpm = this._getBpmByBeat(beat, timingEvents); // (if there are no mid-note BPM changes)
        const noteDuration = this._getNoteDuration(beat, subdivision);

        const timestamp = cursor;
        cursor += noteDuration;
        const eventsByType = this._getEventsByType(line);

        return _(eventsByType)
          .map(({ type, arrows }) => {
            const activeArrows = _.range(
              0,
              this.header.isDouble ? 10 : 5
            ).map((id) => _.includes(arrows, id));

            const id = currentId++;
            const holdArrowsMetadata = this._getHoldArrowsMetadata(
              id,
              beat,
              timestamp,
              type,
              activeArrows,
              holdArrows
            );
            const complexity = this._getComplexityOf(
              type,
              bpm,
              subdivision,
              _.sumBy(activeArrows)
            );

            return {
              id,
              beat,
              timestamp,
              type: type === Events.FAKE_TAP ? Events.NOTE : type,
              playerId: 0,
              arrows: activeArrows.slice(0, 5),
              arrows2: this.header.isDouble ? activeArrows.slice(5, 10) : null,
              complexity,
              isFake: type === Events.FAKE_TAP,
              ...holdArrowsMetadata,
            };
          })
          .filter((it) => _.some(it.arrows) || _.some(it.arrows2))
          .value();
      });
    });
  }

  /** Generates all the events from timing segments. */
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
    let speedFactor = 1;
    let scrollFactor = 1;
    let currentScrollEnabled = true;
    let currentScrollTimestamp = 0;
    let currentScrollBeat = 0;

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
            beat,
            timestamp: warpStart,
            type: Events.WARP,
            length: timestamp - warpStart,
          };
        };
        const createSetTempo = (scrollChangeFrames = 0) => {
          return {
            beat,
            timestamp,
            type: Events.SET_TEMPO,
            bpm: currentBpm,
            scrollBpm: currentBpm * speedFactor * scrollFactor,
            scrollChangeFrames,
          };
        };

        switch (type) {
          case Events.SET_TEMPO: {
            if (data.value >= FAST_BPM_WARP) {
              // (fast-bpm warps work like #WARPS=... that teleport the player to the next BPM change)
              if (warpStart === -1) warpStart = timestamp;
              return null;
            }

            const bpmChange = createSetTempo();

            if (warpStart > -1) {
              const warp = createWarp();
              warpStart = -1;
              return [warp, bpmChange];
            } else {
              return bpmChange;
            }
          }
          case Events.SET_SPEED: {
            speedFactor = data.value;
            const scrollChangeFrames =
              (data.param2 === 0
                ? data.param1 * beatLength
                : data.param1 * SECOND) / FRAME_MS;

            return createSetTempo(scrollChangeFrames);
          }
          case Events.SET_TICKCOUNT: {
            return {
              beat,
              timestamp,
              type,
              tickcount: data.value,
            };
          }
          case Events.SET_FAKE: {
            return {
              beat,
              timestamp,
              type,
              endTime: timestamp + data.value * beatLength,
            };
          }
          case Events.STOP: {
            const length = data.value * SECOND;
            const stop = {
              beat,
              timestamp,
              type,
              length,
              async: false,
            };

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

            // (#SCROLLS with a value > 1 are combined with #SPEEDS)
            const newScrollFactor = Math.max(data.value, 1);
            const events = [];
            if (newScrollFactor != scrollFactor) {
              scrollFactor = newScrollFactor;
              events.push(createSetTempo());
            }

            if (!currentScrollEnabled && scrollEnabled) {
              const length = timestamp - currentScrollTimestamp;
              currentScrollEnabled = true;

              return [
                ...events,
                {
                  beat: currentScrollBeat,
                  timestamp: currentScrollTimestamp,
                  type,
                  length,
                  lengthBeats: beat - currentScrollBeat,
                },
              ];
            }

            if (currentScrollEnabled && !scrollEnabled) {
              currentScrollEnabled = false;
              currentScrollTimestamp = timestamp;
              currentScrollBeat = beat;
            }

            return events;
          }
          case Events.WARP: {
            const length = this._getRangeDuration(beat, beat + data.value);

            return {
              beat,
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

  /**
   * Applies fake taps (F notes) and fake segments (#FAKES:...=...).
   * Fake taps are compiled to: SET_FAKE=1, {note}, SET_FAKE=0
   * Fake segments initially have length, but they are compiled to: SET_FAKE=1, ...{notes}..., SET_FAKE=0
   */
  _applyFakes(events) {
    let fakeEndTime = -1;

    return _.flatMap(events, (it) => {
      let event = it;

      if (it.isFake && fakeEndTime == -1) {
        return [
          {
            id: event.id,
            beat: event.beat,
            timestamp: it.timestamp,
            type: Events.SET_FAKE,
            fakeTap: true,
            enabled: 1,
          },
          event,
          {
            id: event.id,
            beat: event.beat,
            timestamp: it.timestamp,
            type: Events.SET_FAKE,
            fakeTap: true,
            enabled: 0,
          },
        ];
      }

      if (it.type === Events.SET_FAKE) {
        fakeEndTime = it.endTime;

        event = {
          id: it.id,
          beat: it.beat,
          timestamp: it.timestamp,
          type: Events.SET_FAKE,
          enabled: 1,
        };
      }

      if (fakeEndTime !== -1 && it.timestamp >= fakeEndTime) {
        fakeEndTime = -1;

        return [
          {
            id: it.id,
            beat: it.beat,
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

  /**
   * Applies async stops (#SCROLLS=...=0), by converting them to actual STOP events.
   * As STOP events are blocking, all subsequent events must be moved to compensate the stop.
   * The only exceptions are SET_TEMPO/SET_TICKCOUNT, which are processed even if the chart is stopped.
   * #SCROLLS are always optional and used in gimmick charts, so if this conversion can result
   * in a broken chart, the async stop will be ignored.
   * STOP events converted from async stops are always judgeable.
   * This method also calculates/assigns the duration of HOLD_START events.
   */
  _applyAsyncStopsAndAddHoldLengths(events) {
    let stoppedTime = 0;
    let lastStop = null;

    const eventsById = {};

    return this._sort(
      _(events)
        .map((it, i) => {
          if (it.type === Events.STOP_ASYNC) {
            if (lastStop != null || !this._canAsyncStopBeApplied(events, it, i))
              return null;
            const timestamp = it.timestamp - stoppedTime;

            return (lastStop = {
              ...it,
              timestamp,
              type: Events.STOP,
              async: true,
              asyncStoppedTime: stoppedTime + it.length,
            });
          } else if (lastStop != null) {
            if (it.beat > lastStop.beat) {
              stoppedTime += lastStop.length;
              lastStop = null;
            }
          }

          let timestamp = it.timestamp - stoppedTime;

          if (
            it.type === Events.SET_TEMPO ||
            it.type === Events.SET_TICKCOUNT
          ) {
            // don't move these events
            timestamp = it.timestamp;
          } else if (it.type === Events.HOLD_END && it.holdStartIds != null) {
            // ensure HOLD_ENDs won't be moved before their HOLD_STARTs
            timestamp = Math.max(
              _.max(
                it.holdStartIds
                  .filter((it) => it != null)
                  .map((id) => eventsById[id])
                  .filter((it) => it != null)
                  .map((it) => it.timestamp)
              ),
              timestamp
            );

            for (let holdStartId of it.holdStartIds) {
              if (holdStartId != null) {
                const holdStart = eventsById[holdStartId];
                holdStart.length = timestamp - holdStart.timestamp;
              }
            }
          }

          const event = { ...it, timestamp };
          eventsById[event.id] = event;
          return event;
        })
        .compact()
        .value()
    );
  }

  /**
   * When there are STOPs and WARPs in the same beat, they should be processed in the order
   * <STOP-WARP> instead of <WARP-STOP>, no matter what type of STOP the event is (async or regular).
   * This function sorts these events.
   */
  _sortStopWarps(events) {
    return this._sort(
      _(events)
        .groupBy((it) => Math.round(it.timestamp))
        .flatMap((subEvents, timestampStr) => {
          const warps = subEvents.filter((it) => it.type === Events.WARP);
          const stops = subEvents.filter((it) => it.type === Events.STOP);
          if (warps.length > 1)
            throw new Error("multiple_warps_in_timestamp: " + timestampStr);
          if (stops.length > 1)
            throw new Error("multiple_stops_in_timestamp: " + timestampStr);

          if (warps.length === 1 && stops.length === 1) {
            const warp = warps[0];
            const stop = stops[0];
            const others = subEvents.filter(
              (it) => it.type !== Events.WARP && it.type !== Events.STOP
            );

            return [
              ...others,
              { priority: 1, ...stop },
              { priority: 2, ...warp },
            ];
          }

          return subEvents;
        })
        .value()
    );
  }

  /**
   * Determines whether an async stop can be applied or not.
   * As "applying" means moving all subsequent events, this only returns true
   * if no events would be placed before song's start, and if there are no WARP
   * or actual STOP events inside.
   */
  _canAsyncStopBeApplied(events, asyncStop, i) {
    let nextMovableEvents = events.slice(i + 1);
    const nextAsyncStopIndex = _.findIndex(
      nextMovableEvents,
      (ev) => ev.type === Events.STOP_ASYNC
    );
    if (nextAsyncStopIndex > -1)
      nextMovableEvents = nextMovableEvents
        .slice(0, nextAsyncStopIndex)
        .filter((ev) => ev.beat > asyncStop.beat);

    const eventsBeforeSongStart = nextMovableEvents.some(
      (ev) => ev.timestamp - asyncStop.length < 0
    );
    const warpsOrStopsDuringAsyncStop = nextMovableEvents.some(
      (ev) =>
        (ev.type === Events.WARP || ev.type === Events.STOP) &&
        ev.beat < asyncStop.beat + asyncStop.durationBeats
    );

    return !eventsBeforeSongStart && !warpsOrStopsDuringAsyncStop;
  }

  /** Moves all the events to adjust the chart's offset. */
  _applyOffset(events) {
    return events.map((it) => ({
      ...it,
      timestamp: this.header.offset + it.timestamp,
    }));
  }

  /** Sorts events by timestamp => type => id. */
  _sort(events) {
    return _.sortBy(events, [
      (it) => Math.round(it.timestamp),
      (it) => it.priority || 0,
      (it) =>
        // (fake taps are compiled to: SET_FAKE=1, {note}, SET_FAKE=0)
        it.type === Events.SET_FAKE && it.fakeTap
          ? it.enabled
            ? 0.5
            : 1.5
          : // (STOPs should be *before* WARPs, async STOPs should be *after* WARPs)
          it.type === Events.STOP
          ? it.async
            ? 7
            : 5.5
          : it.type,
      (it) => it.id,
    ]);
  }

  /** Returns a list of measures (raw data). */
  _getMeasures() {
    return this.content
      .split(",")
      .map((it) => it.trim())
      .filter(_.identity);
  }

  /** Returns the parsed lines within a measure. */
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
      ) // (weird f2 event syntax)
      .map((it) => it.replace(/[MK]/g, "0")) // (ignored SSC events)
      .filter((it) => {
        const isValid = (this.header.isDouble
          ? NOTE_DATA_DOUBLE
          : NOTE_DATA_SINGLE
        ).test(it);

        if (!isValid) throw new Error("invalid_line: " + it);
        return true;
      });
  }

  /** Separates events by their type. */
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

  /** Returns the beat length of a BPM in milliseconds. */
  _getBeatLengthByBpm(bpm) {
    if (bpm === 0) return 0;
    return MINUTE / bpm;
  }

  /** Returns the BPM of a certain beat. */
  _getBpmByBeat(beat) {
    const bpm = _.findLast(this._getFiniteBpms(), (bpm) => beat >= bpm.key);
    if (!bpm) return 0;

    return bpm.value;
  }

  /** Calculates the duration of a note, taking into account mid-note BPM changes. */
  _getNoteDuration(beat, subdivision) {
    const startBeat = beat;
    const durationBeats = BEAT_UNIT * subdivision;
    const endBeat = startBeat + durationBeats;

    const bpms = this._getFiniteBpms();
    let currentBpm = this._getBpmByBeat(startBeat);
    let currentBeat = startBeat;
    let durationMs = 0;
    let completion = 0;

    for (let bpm of bpms) {
      if (bpm.key > startBeat && bpm.key < endBeat) {
        // durationBeats      -> 1
        // beatsInPreviousBpm -> fraction

        const beatsInPreviousBpm = bpm.key - currentBeat;
        const fraction = beatsInPreviousBpm / durationBeats;
        durationMs += this._getBeatLengthByBpm(currentBpm) * beatsInPreviousBpm;
        completion += fraction;

        currentBpm = bpm.value;
        currentBeat = bpm.key;
      }
    }

    if (completion < 1) {
      durationMs +=
        this._getBeatLengthByBpm(currentBpm) * durationBeats * (1 - completion);
      completion = 1;
    }

    return durationMs;
  }

  /** Calculates the duration of a range, in beats. */
  _getRangeDuration(startBeat, endBeat, precision = SEMISEMIFUSE) {
    let length = 0;

    for (let beat = startBeat; beat < endBeat; beat += precision) {
      const bpm = this._getBpmByBeat(beat);
      length += this._getBeatLengthByBpm(bpm) * precision;
    }

    return length;
  }

  /** Return finite BPM changes, ignoring fast-bpm warps. */
  _getFiniteBpms() {
    return this.header.bpms.filter((it) => it.value <= FAST_BPM_WARP);
  }

  /** Connects HOLD_STARTs and HOLD_ENDs. */
  _getHoldArrowsMetadata(id, beat, timestamp, type, activeArrows, holdArrows) {
    let metadata = null;
    if (type === Events.HOLD_START) {
      for (let i = 0; i < activeArrows.length; i++) {
        if (activeArrows[i]) {
          if (holdArrows[i] == null) {
            holdArrows[i] = id;
          } else {
            throw new Error(`unbalanced_hold_arrow: ${beat}/${timestamp}`);
          }
        }
      }
    } else if (type === Events.HOLD_END) {
      for (let i = 0; i < activeArrows.length; i++) {
        if (activeArrows[i]) {
          if (holdArrows[i] != null) {
            if (metadata == null) metadata = { holdStartIds: [] };
            metadata.holdStartIds[i] = holdArrows[i];
            holdArrows[i] = null;
          } else {
            throw new Error(`orphan_hold_arrow: ${beat}/${timestamp}`);
          }
        }
      }
    }

    return metadata;
  }

  /** Returns the complexity index of a note. */
  _getComplexityOf(type, bpm, subdivision, arrowCount) {
    const isHold = type === Events.HOLD_START;
    const isJump = arrowCount > 1;

    return (type === Events.NOTE || isHold) &&
      this.lastTimestamp < MAX_TIMESTAMP
      ? ((1 - subdivision) *
          Math.log(bpm) *
          (isJump ? Math.log2(2 + arrowCount) : 1) *
          (isHold ? 1.3 : 1)) /
          (this.lastTimestamp / SECOND)
      : null;
  }

  /**
   * This is used for the complexity index.
   * If #LASTSECONDHINT is not provided, last event's timestamp will be used.
   */
  _calculateLastTimestamp() {
    this.lastTimestamp = MAX_TIMESTAMP;

    try {
      this.lastTimestamp =
        this.metadata.lastMillisecond < MAX_TIMESTAMP
          ? this.metadata.lastMillisecond
          : _.last(this.events).timestamp;
    } catch (e) {}
  }

  /** Validates the chart. */
  _validate() {
    const bpms = this._getFiniteBpms();
    let lastBeat = -1;
    for (let bpm of bpms) {
      if (Math.abs(bpm.key - lastBeat) < SEMISEMIFUSE)
        throw new Error("bpm_change_too_fast:\n    " + JSON.stringify(bpm));
      lastBeat = bpm.key;
    }
  }
};

const SECOND = 1000;
const MINUTE = 60 * SECOND;
const FRAME_MS = 17;
const BEAT_UNIT = 4;
const FAST_BPM_WARP = 999999;
const NOTE_DATA_SINGLE = /^[\dF][\dF][\dF][\dF][\dF]$/;
const NOTE_DATA_DOUBLE = /^[\dF][\dF][\dF][\dF][\dF][\dF][\dF][\dF][\dF][\dF]$/;
const SEMISEMIFUSE = 1 / 128;
const MAX_TIMESTAMP = 3600000;
