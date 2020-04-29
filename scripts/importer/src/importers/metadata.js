const Simfile = require("../parser/Simfile");
const SongSerializer = require("../serializer/SongSerializer");
const fs = require("fs");
const $path = require("path");
const utils = require("../utils");
const _ = require("lodash");

const EXTENSION = "pius";

module.exports = (name, filePath, outputPath) => {
  const content = fs.readFileSync(filePath).toString();
  const { metadata, charts } = new Simfile(content, name);

  const levels = charts.map((it) => `${it.header.name}[${it.header.level}]`);
  console.log("-> levels: ".bold + `(${levels.join(", ")})`.cyan);
  const levelNames = charts.map((it) => it.header.name);
  const normal = utils.insistentChoice("Which one is NORMAL?", levelNames);
  const hard = utils.insistentChoice("Which one is HARD?", levelNames);
  const crazy = utils.insistentChoice("Which one is CRAZY?", levelNames);
  const channel = utils.insistentChoice(
    "What channel? (0 = ORIGINAL, 1 = KPOP, 2 = WORLD)",
    _.range(3)
  );
  const writeChanges = utils.insistentChoice("Save changes? (y/n)", ["y", "n"]);

  const simfile = { metadata, charts };
  const output = new SongSerializer(simfile).serialize();

  fs.writeFileSync($path.join(outputPath, `${name}.${EXTENSION}`), output);
};
