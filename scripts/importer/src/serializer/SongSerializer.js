const Protocol = require("bin-protocol");
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
      .String(_.padEnd(metadata.title, TITLE_LEN))
      .String(_.padEnd(metadata.artist, TITLE_LEN))
      .UInt8(2) // TODO: Unhardcode channel
      .UInt32LE(metadata.sampleStart)
      .UInt32LE(metadata.sampleLength)
      .ChartArray(charts).result;
  }

  _defineTypes() {
    this.protocol.define("String", {
      write: function (string) {
        const characters = string.split("").map((char) => char.charCodeAt(0));
        this.loop(characters, this.UInt8).UInt8(0);
      },
    });

    this.protocol.define("Chart", {
      write: function (chart) {
        this.UInt8(2) // TODO: Unhardcode difficulty
          .UInt8(chart.header.level)
          .EventArray(chart.events);
      },
    });

    this.protocol.define("Event", {
      write: function (event) {
        const data = _.range(0, 5).reduce(
          (acum, elem) => acum | (event.arrows[elem] ? ARROW_MASKS[elem] : 0),
          event.type
        );
        this.UInt32LE(event.timestamp).UInt8(data);
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

const TITLE_LEN = 40 - 1;
const ARTIST_LEN = 15 - 1;
// (-1 for \0)

const ARROW_MASKS = [
  0b00001000,
  0b00010000,
  0b00100000,
  0b01000000,
  0b10000000,
];
