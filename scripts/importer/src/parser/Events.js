const events = {
  NOTE: 0,
  HOLD_START: 1,
  HOLD_TAIL: 2,
  STOP: 3,
  SET_TEMPO: 4,
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
        return events.HOLD_TAIL;
      default:
        return null;
    }
  },
};
