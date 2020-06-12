const Protocol = require("bin-protocol");
const Events = require("../parser/Events");
const DifficultyLevels = require("../parser/DifficultyLevels");
const Channels = require("../parser/Channels");
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
        const padding = size - string.length;
        for (let i = 0; i < padding; i++) this.UInt8(0);
      },
    });

    this.protocol.define("Chart", {
      write: function (chart) {
        this.UInt8(DifficultyLevels[chart.header.difficulty])
          .UInt8(chart.header.level)
          .EventArray(chart.events);
      },
    });

    this.protocol.define("Event", {
      write: function (event) {
        this.Int32LE(event.timestamp);

        if (event.type === Events.SET_TEMPO)
          this.UInt8(event.type)
            .UInt32LE(normalizeUInt(event.bpm))
            .UInt32LE(normalizeUInt(event.scrollBpm))
            .UInt32LE(normalizeUInt(event.scrollChangeFrames));
        else if (event.type === Events.SET_TICKCOUNT)
          this.UInt8(event.type).UInt32LE(normalizeUInt(event.tickcount));
        else if (event.type === Events.SET_FAKE)
          this.UInt8(event.type).UInt32LE(event.enabled ? 1 : 0);
        else if (event.type === Events.STOP || event.type === Events.STOP_ASYNC)
          this.UInt8(Events.STOP).UInt32LE(normalizeUInt(event.length));
        else if (event.type === Events.WARP)
          this.UInt8(event.type).UInt32LE(normalizeUInt(event.length));
        else {
          const data = _.range(0, 5).reduce(
            (acum, elem) => acum | (event.arrows[elem] ? ARROW_MASKS[elem] : 0),
            event.type
          );
          this.UInt8(data);
        }
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

const normalizeUInt = (number) => {
  if (number === Infinity || number > INFINITY) return INFINITY;
  return Math.round(number);
};

const TITLE_LEN = 31;
const ARTIST_LEN = 27;
const INFINITY = 0xffffffff;

const ARROW_MASKS = [
  0b00001000,
  0b00010000,
  0b00100000,
  0b01000000,
  0b10000000,
];
