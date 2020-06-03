const events = {
  NOTE: 0,
  HOLD_START: 1,
  HOLD_END: 2,
  SET_TEMPO: 3,
  SET_SCROLL: 4,
  SET_TICKCOUNT: 5,
  STOP: 6,
  WARP: 7,
};

module.exports = {
  ...events,
  parse(note) {
    switch (note) {
      case "1":
        return events.NOTE;
      case "2":
        return events.HOLD_START;
      case "3":
        return events.HOLD_END;
      default:
        return null;
    }
  },
};
