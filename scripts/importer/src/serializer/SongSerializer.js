const Protocol = require("bin-protocol");
const Events = require("../parser/Events");
const DifficultyLevels = require("../parser/DifficultyLevels");
const Channels = require("../parser/Channels");
const Mods = require("../parser/Mods");
const _ = require("lodash");

module.exports = class SongSerializer {
  constructor(simfile) {
    this.simfile = simfile;
    this.protocol = new Protocol();

    this._defineTypes();
  }

  serialize() {
    const buffer = this.protocol.write();
    const { metadata, charts } = this.simfile;

    return buffer
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
        const eventChunkSize = _.sumBy(
          chart.events,
          (it) => 4 /* (timestamp) */ + EVENT_SERIALIZERS.get(it).size
        );

        this.UInt8(DifficultyLevels[chart.header.difficulty])
          .UInt8(chart.header.level)
          .UInt32LE(4 /* (eventCount) */ + eventChunkSize)
          .EventArray(chart.events);
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

const EVENT_SERIALIZERS = {
  get(event) {
    return this[event.type] || this.NOTES;
  },
  NOTES: {
    write: function (event) {
      const data = _.range(0, 5).reduce(
        (acum, elem) => acum | (event.arrows[elem] ? ARROW_MASKS[elem] : 0),
        event.type
      );
      this.UInt8(data);
    },
    size: 1,
  },
  [Events.SET_FAKE]: {
    write: function (event) {
      this.UInt8(event.type).UInt32LE(event.enabled ? 1 : 0);
    },
    size: 1 + 4,
  },
  [Events.SET_TEMPO]: {
    write: function (event) {
      this.UInt8(event.type)
        .UInt32LE(normalizeUInt(event.bpm))
        .UInt32LE(normalizeUInt(event.scrollBpm))
        .UInt32LE(normalizeUInt(event.scrollChangeFrames));
    },
    size: 1 + 4 + 4 + 4,
  },
  [Events.SET_TICKCOUNT]: {
    write: function (event) {
      this.UInt8(event.type).UInt32LE(normalizeUInt(event.tickcount));
    },
    size: 1 + 4,
  },
  [Events.STOP]: {
    write: function (event) {
      this.UInt8(Events.STOP)
        .UInt32LE(normalizeUInt(event.length))
        .UInt32LE(event.judgeable ? 1 : 0);
    },
    size: 1 + 4 + 4,
  },
  [Events.WARP]: {
    write: function (event) {
      this.UInt8(event.type).UInt32LE(normalizeUInt(event.length));
    },
    size: 1 + 4,
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
