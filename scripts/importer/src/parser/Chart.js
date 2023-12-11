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
   * Note events are specifically NOTE, HOLD_START and HOLD_END
   */
  get events() {
    const timingEvents = this._getTimingEvents().map((it, i) => ({
      id: 1 + i,
      ...it,
    }));
    const noteEvents = this._getNoteEvents(timingEvents);

    return this._applyOffset(
      this._applyAsyncStopsAndAddHoldLengths(
        this._applyFakes(this._sort([...timingEvents, ...noteEvents]))
      )
    );
  }

  /** Generates events specifically from note data. */
  _getNoteEvents(timingEvents) {
    let currentId = 0;
    const measureGroups = this._getMeasureGroups();

    return _(measureGroups).flatMap((measures) => {
      let cursor = 0;
      const holdArrows = [];

      return _.flatMap(measures, (measure, measureIndex) => {
        // 1 measure = 1 whole note = BEAT_UNIT beats
        const lines = this._getMeasureLines(measure);
        const subdivision = 1 / lines.length;

        return _.flatMap(lines, (line, noteIndex) => {
          const beat = (measureIndex + noteIndex * subdivision) * BEAT_UNIT;
          const bpm = this._getBpmByBeat(beat, timingEvents); // (this is an approximation for `complexity`, only valid if there are no mid-note BPM changes)
          const noteDuration = this._getNoteDuration(beat, subdivision); // (this calculates the actual note duration)

          const timestamp = cursor;
          cursor += noteDuration;
          const eventsByType = this._getEventsByType(line);

          return _(eventsByType)
            .map(({ type, arrows }) => {
              const activeArrows = _.range(
                0,
                this.header.isDouble ? 10 : 5
              ).map((i) => _.includes(arrows, i));

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
                arrows2: this.header.isDouble
                  ? activeArrows.slice(5, 10)
                  : null,
                complexity,
                isFake: type === Events.FAKE_TAP,
                ...holdArrowsMetadata,
              };
            })
            .filter((it) => _.some(it.arrows) || _.some(it.arrows2))
            .value();
        });
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

        if (data.value < 0 && type !== Events.SET_SPEED) {
          throw new Error(
            "invalid_negative_timing_segment:\n    " +
              JSON.stringify({ type, data })
          );
        }

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
          const scrollBpm = currentBpm * speedFactor * scrollFactor;

          return {
            beat,
            timestamp,
            type: Events.SET_TEMPO,
            bpm: currentBpm,
            scrollBpm,
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
            if (data.value < 0) return null; // (negative speed segments are ignored)

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
            const newScrollFactor = data.value;
            const scrollEnabled = newScrollFactor > 0;

            // (#SCROLLS with a value > 0 are combined with #SPEEDS)
            const events = [];
            if (scrollEnabled && newScrollFactor != scrollFactor) {
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
   * Applies fake segments (#FAKES:...=... or SET_FAKE events here). This adds `isFake: true`
   * to each event inside a fake segment. Fake taps (F notes) already have this property.
   * Then, SET_FAKE events are removed, the serializer will use the fake flag of the events instead.
   */
  _applyFakes(events) {
    let fakeEndTime = -1;

    return _.flatMap(events, (it) => {
      if (it.type === Events.SET_FAKE) {
        fakeEndTime = it.endTime;
        return [];
      }

      if (fakeEndTime !== -1 && it.timestamp >= fakeEndTime) fakeEndTime = -1;
      if (fakeEndTime !== -1) it.isFake = true;

      return it;
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
      (it) => it.type,
      (it) => it.id,
    ]);
  }

  /**
   * Returns a list of measure groups (raw data).
   * This tipically returns a list with only one item (a list with all the measures).
   * In co-op charts, there's a divider ("&") that creates a <sub-chart> for each player.
   * We'll keep each group separated so the events are created without cross-connecting arrows from other players.
   */
  _getMeasureGroups() {
    const hasDivider = _.includes(this.content, "&");
    if (hasDivider && !this.header.isMultiplayer)
      throw new Error("unexpected_divider_in_single_player_chart");

    const subCharts = _(this.content.split("&"));

    return subCharts.map((subChart) =>
      subChart
        .split(",")
        .map((it) => it.trim())
        .filter(_.identity)
    );
  }

  /** Returns the parsed lines within a measure. */
  _getMeasureLines(measure) {
    let divider = false;

    return (
      measure
        .split(/\r?\n/)
        .map((it) => it.replace(/\/\/.*/g, ""))
        .map((it) => it.trim())
        .filter((it) => !_.isEmpty(it))
        .map((it) =>
          it.replace(/{(\w)\|\w\|(\w)\|\w}/g, (__, note, fake) =>
            fake === "1" ? "F" : note
          )
        ) // advanced note syntax: {note type|appearance|fake flag|noteskin override}
        // ^^^ only <note type> and <fake flag> are supported
        .map((it) => it.replace(/[MKSVH]/g, "0")) // ignored SSC events: Mine, AutoKeySound, Sudden, Vanish, Hidden
        .map((it) => {
          if (/[XxYyZz]/.test(it)) {
            this.header.isMultiplayer = true;
            this.header.levelStr = `m${this.header.level}`;
          }

          return it
            .replace(/X/g, "1")
            .replace(/x/g, "2")
            .replace(/Y/g, "1")
            .replace(/y/g, "2")
            .replace(/Z/g, "1")
            .replace(/z/g, "2");
        }) // co-op charts note types
        .filter((it) => {
          if (divider || (this.header.isMultiplayer && it === "&")) {
            divider = true;
            return false;
          }
          return true;
        }) // ignore "&" sections
        .filter((it) => {
          const isValid = (this.header.isDouble
            ? NOTE_DATA_DOUBLE
            : NOTE_DATA_SINGLE
          ).test(it);

          if (!isValid) throw new Error("invalid_line: " + it);
          return true;
        })
    );
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
            // (this is allowed as it's usually not terrible)
            // throw new Error(`orphan_hold_arrow: ${beat}/${timestamp}`);
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
