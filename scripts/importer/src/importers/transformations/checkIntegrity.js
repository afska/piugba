const _ = require("lodash");

module.exports = (metadata, charts, filePath) => {
  if (_.isEmpty(metadata.title))
    throw new Error(`missing_title: \`${filePath}\``);
  if (_.isEmpty(metadata.artist))
    throw new Error(`missing_artist: \`${filePath}\``);
  if (!_.isFinite(metadata.sampleStart))
    throw new Error(`invalid_samplestart: \`${filePath}\``);
  if (!_.isFinite(metadata.sampleLength))
    throw new Error(`invalid_samplelength: \`${filePath}\``);
  if (_.isEmpty(charts)) throw new Error(`no_charts: \`${filePath}\``);
};
