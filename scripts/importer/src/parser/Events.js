const events = {
  // Actual events:
  NOTE: 0,
  HOLD_START: 1,
  HOLD_END: 2,
  SET_TEMPO: 3,
  SET_TICKCOUNT: 4,
  STOP: 5,
  WARP: 6,
  // Temporary events:
  FAKE_TAP: 100,
  SET_SPEED: 103,
  STOP_ASYNC: 105,
  SET_FAKE: 200,
};

module.exports = {
  ...events,
  isNote(type) {
    return type === events.NOTE || type === events.HOLD_START;
  },
  parse(note) {
    switch (note) {
      case "1":
        return events.NOTE;
      case "2":
        return events.HOLD_START;
      case "3":
        return events.HOLD_END;
      case "F":
        return events.FAKE_TAP;
      default:
        return null;
    }
  },
};
