const Protocol = require("bin-protocol");
const Events = require("../parser/Events");
const DifficultyLevels = require("../parser/DifficultyLevels");
const Channels = require("../parser/Channels");
const Mods = require("../parser/Mods");
const _ = require("lodash");

let TMP = {};

module.exports = class SongSerializer {
  constructor(simfile) {
    this.simfile = simfile;
    this.protocol = new Protocol();

    this._defineTypes();
  }

  serialize() {
    TMP = {};

    const buffer = this.protocol.write();
    const { id, metadata, charts } = this.simfile;

    return buffer
      .UInt8(id)
      .String(metadata.title, TITLE_LEN)
      .String(metadata.artist, ARTIST_LEN)
      .UInt8(Channels[metadata.channel])
      .UInt32LE(metadata.lastMillisecond)
      .UInt32LE(metadata.sampleStart)
      .UInt32LE(metadata.sampleLength)
      .Config(metadata.config)
      .ChartArray(charts).result;
  }

  _defineTypes() {
    this.protocol.define("String", {
      write: function (string, size) {
        const characters = string
          .substring(0, size - 1)
          .split("")
          .map((char) => char.charCodeAt(0));

        this.loop(characters, this.UInt8);
        const padding = size - characters.length;
        for (let i = 0; i < padding; i++) this.UInt8(0);
      },
    });

    this.protocol.define("Config", {
      write: function (config) {
        config.APPLY_TO.forEach((it) => this.UInt8(it));
        this.UInt8(config.IS_BOSS);

        _.forEach(Mods, (v, k) => {
          this.UInt8(v[config[k]]);
        });

        const hasMessage = config.MESSAGE !== "";
        this.UInt8(hasMessage);
        if (hasMessage) this.String(config.MESSAGE, MESSAGE_LEN);
      },
    });

    this.protocol.define("Chart", {
      write: function (chart) {
        TMP.chart = chart;

        const events = chart.events;
        const [rythmEvents, normalEvents] = _.partition(
          events,
          (it) =>
            it.type === Events.SET_TEMPO || it.type === Events.SET_TICKCOUNT
        );

        const eventChunkSize = _.sumBy(
          events,
          (it) => 4 /* (timestamp) */ + EVENT_SERIALIZERS.get(it).size(it)
        );

        this.UInt8(DifficultyLevels[chart.header.difficulty])
          .UInt8(chart.header.level)
          .UInt8(chart.header.isMultiplayer ? 2 : chart.header.isDouble ? 1 : 0)
          .UInt32LE(4 * 2 /* (eventCounts) */ + eventChunkSize)
          .EventArray(rythmEvents)
          .EventArray(normalEvents);
      },
    });

    this.protocol.define("Event", {
      write: function (event) {
        this.Int32LE(normalizeUInt(event.timestamp));

        const { write } = EVENT_SERIALIZERS.get(event);
        write.bind(this)(event);
      },
    });

    this.protocol.define("ChartArray", {
      write: function (charts) {
        this.UInt8(charts.length).loop(charts, this.Chart);
      },
    });

    this.protocol.define("EventArray", {
      write: function (events) {
        this.UInt32LE(events.length).loop(events, this.Event);
      },
    });
  }
};

const SERIALIZE_ARROWS = (arrows) =>
  _.range(0, 5).reduce(
    (accum, elem) => accum | (arrows[elem] ? ARROW_MASKS[elem] : 0),
    0
  );

const EVENT_SERIALIZERS = {
  get(event) {
    return this[event.type] || this.NOTES;
  },
  NOTES: {
    write: function (event) {
      this.UInt8(SERIALIZE_ARROWS(event.arrows) | event.type);
      if (event.arrows2) this.UInt8(SERIALIZE_ARROWS(event.arrows2));
      if (event.type === Events.HOLD_START)
        this.UInt32LE(event.length != null ? normalizeUInt(event.length) : 0);
    },
    size: (event) =>
      (event.arrows2 ? 1 + 1 : 1) + (event.type === Events.HOLD_START ? 4 : 0),
  },
  [Events.SET_FAKE]: {
    write: function (event) {
      this.UInt8(event.type).UInt32LE(event.enabled ? 1 : 0);
    },
    size: () => 1 + 4,
  },
  [Events.SET_TEMPO]: {
    write: function (event) {
      const chart = TMP?.chart;
      if (chart == null) throw new Error("serializer_chart_not_found");

      function findDominantScrollBpm(events) {
        if (events.length === 0) return 0;

        const finalTimestamp = _.last(chart.events).timestamp;
        let bpmDurations = {};
        events.forEach((item, index) => {
          const nextTimestamp =
            index < events.length - 1
              ? events[index + 1].timestamp
              : finalTimestamp;
          const length = nextTimestamp - item.timestamp;

          if (!bpmDurations[item.scrollBpm]) bpmDurations[item.scrollBpm] = 0;
          bpmDurations[item.scrollBpm] += length;
        });

        let maxDuration = 0;
        let dominantBpm = null;

        _.forEach(bpmDurations, (duration, bpm) => {
          if (duration > maxDuration) {
            maxDuration = duration;
            dominantBpm = parseInt(bpm);
          }
        });

        return dominantBpm || 0;
      }

      const dominantScrollBpm = findDominantScrollBpm(
        chart.events.filter((it) => it.type === Events.SET_TEMPO)
      );
      const autoVelocityFactor = Math.max(
        event.scrollBpm / dominantScrollBpm,
        0.25
      );

      const scrollBpm = normalizeUInt(event.scrollBpm);
      let scrollChangeFrames = normalizeUInt(event.scrollChangeFrames);
      if (scrollChangeFrames === INFINITY) scrollChangeFrames = 0;
      const composedScrollBpm = new Uint32Array(1);
      composedScrollBpm[0] =
        scrollBpm === INFINITY
          ? scrollBpm
          : (((scrollBpm & 0xffff) << 16) | scrollChangeFrames) & 0xffffffff;

      this.UInt8(event.type)
        .UInt32LE(normalizeUInt(event.bpm))
        .UInt32LE(composedScrollBpm[0])
        .UInt32LE(
          autoVelocityFactor >= 1 || autoVelocityFactor === 0
            ? 1
            : INFINITY * autoVelocityFactor
        );
    },
    size: () => 1 + 4 + 4 + 4,
  },
  [Events.SET_TICKCOUNT]: {
    write: function (event) {
      this.UInt8(event.type).UInt32LE(normalizeUInt(event.tickcount));
    },
    size: () => 1 + 4,
  },
  [Events.STOP]: {
    write: function (event) {
      this.UInt8(Events.STOP)
        .UInt32LE(normalizeUInt(event.length))
        .UInt32LE(event.async ? 1 : 0)
        .UInt32LE(event.async ? normalizeUInt(event.asyncStoppedTime) : 0);
    },
    size: () => 1 + 4 + 4 + 4,
  },
  [Events.WARP]: {
    write: function (event) {
      this.UInt8(event.type).UInt32LE(normalizeUInt(event.length));
    },
    size: () => 1 + 4,
  },
};

const normalizeUInt = (number) => {
  if (number === Infinity || number > INFINITY) return INFINITY;
  return Math.round(number);
};

const TITLE_LEN = 30 + 1; // +1 = \0;
const ARTIST_LEN = 26 + 1; // +1 = \0;
const MESSAGE_LEN = 25 + 2 + 25 + 2 + 25 + 2 + 25 + 1; // +2 = \r\n ; +1 = \0
const INFINITY = 0xffffffff;

const ARROW_MASKS = [
  0b00001000,
  0b00010000,
  0b00100000,
  0b01000000,
  0b10000000,
];
