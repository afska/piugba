const { exec } = require("child_process");
const fs = require("fs");

const beep = () => exec(`rundll32 user32.dll,MessageBeep`);

const json = fs.readFileSync("./test/test.json");
const data = JSON.parse(json);
const events = data.events;

let cursor = 0;
const processNextNote = () => {
  const note = events.shift();

  setTimeout(() => {
    beep();
    cursor = note.timestamp;
    processNextNote();
  }, note.timestamp - cursor);
};
processNextNote();
