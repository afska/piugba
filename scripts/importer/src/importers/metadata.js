const Simfile = require("../parser/Simfile");
const SongSerializer = require("../serializer/SongSerializer");
const checkIntegrity = require("./integrity/checkIntegrity");
const completeMissingData = require("./integrity/completeMissingData");
const fs = require("fs");
const $path = require("path");
const _ = require("lodash");

const EXTENSION = "pius";

module.exports = (name, filePath, outputPath) => {
  const content = fs.readFileSync(filePath).toString();
  const { metadata, charts } = new Simfile(content, name);

  checkIntegrity(metadata, charts);
  const simfile = completeMissingData(metadata, charts, content, filePath);

  const output = new SongSerializer(simfile).serialize();
  fs.writeFileSync($path.join(outputPath, `${name}.${EXTENSION}`), output);

  return simfile;
};
