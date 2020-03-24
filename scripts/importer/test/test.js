const { exec } = require("child_process");
const fs = require("fs");

const beep = () => exec(`rundll32 user32.dll,MessageBeep`);

const json = fs.readFileSync("./test.json");
const data = JSON.parse(json);

let cursor = 0;

const validEvents = data.events.filter((it) => it.arrows.some((it) => it));

const processNextNote = () => {
  const note = validEvents.shift();

  setTimeout(() => {
    beep();
    cursor = note.timestamp;
    processNextNote();
  }, note.timestamp - cursor);
};

processNextNote();
