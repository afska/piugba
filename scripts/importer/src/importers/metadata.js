const Simfile = require("../parser/Simfile");
const SongSerializer = require("../serializer/SongSerializer");
const fs = require("fs");
const $path = require("path");

const EXTENSION = "pius";

module.exports = (name, filePath, outputPath) => {
  const content = fs.readFileSync(filePath).toString();
  const { metadata, charts } = new Simfile(content, name);

  const simfile = { metadata, charts };
  const output = new SongSerializer(simfile).serialize();

  fs.writeFileSync($path.join(outputPath, `${name}.${EXTENSION}`), output);
  return ` (${simfile.charts.map((it) => it.header.level).join(", ")})`;
};
