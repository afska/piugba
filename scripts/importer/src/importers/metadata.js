const Simfile = require("../parser/Simfile");
const SongSerializer = require("../serializer/SongSerializer");
const checkIntegrity = require("./transformations/checkIntegrity");
const { applyOffsets } = require("./transformations/applyOffsets");
const completeMissingData = require("./transformations/completeMissingData");
const fs = require("fs");
const $path = require("path");
const _ = require("lodash");

const EXTENSION = "pius";
const JSON_EXTENSION = "json";

module.exports = (name, filePath, outputPath, id, prefix = null) => {
  let content = fs.readFileSync(filePath).toString();
  if (prefix != null) content = prefix + "\r\n" + content;
  const { metadata, charts } = new Simfile(content);

  checkIntegrity(metadata, charts, filePath);
  applyOffsets(metadata, charts);
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
