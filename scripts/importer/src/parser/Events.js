const events = {
  NOTE: 0,
  HOLD_START: 1,
  HOLD_TICK: 2,
  HOLD_END: 3,
  SET_TEMPO: 4,
  STOP: 5,
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
