const Simfile = require("../parser/Simfile");
const SongSerializer = require("../serializer/SongSerializer");
const checkIntegrity = require("./integrity/checkIntegrity");
const completeMissingData = require("./integrity/completeMissingData");
const fs = require("fs");
const $path = require("path");
const _ = require("lodash");

const EXTENSION = "pius";
const JSON_EXTENSION = "json";

module.exports = (name, filePath, outputPath, id) => {
  const content = fs.readFileSync(filePath).toString();
  const { metadata, charts } = new Simfile(content, name);

  checkIntegrity(metadata, charts);
  const simfile = completeMissingData(metadata, charts, content, filePath);
  simfile.id = id;

  if (GLOBAL_OPTIONS.json) {
    fs.writeFileSync(
      $path.join(outputPath, `${name}.${JSON_EXTENSION}`),
      JSON.stringify(simfile, null, 2)
    );
  }

  const output = new SongSerializer(simfile).serialize();
  fs.writeFileSync($path.join(outputPath, `${name}.${EXTENSION}`), output);

  return simfile;
};
