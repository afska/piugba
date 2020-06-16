const events = {
  SET_FAKE: 0,
  NOTE: 1,
  HOLD_START: 2,
  HOLD_END: 3,
  SET_TEMPO: 4,
  SET_TICKCOUNT: 5,
  STOP: 6,
  WARP: 7,
  SET_SPEED: 104,
  STOP_ASYNC: 106,
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
