const events = {
  NOTE: 0,
  HOLD_START: 1,
  HOLD_END: 2,
  SET_TEMPO: 3,
  SET_TICKCOUNT: 4,
  STOP: 5,
  WARP: 6,
  SET_SPEED: 103,
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
