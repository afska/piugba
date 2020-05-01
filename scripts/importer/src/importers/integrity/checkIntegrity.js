const _ = require("lodash");

module.exports = (metadata, charts) => {
  if (_.isEmpty(metadata.id)) throw new Error("missing_id");
  if (_.isEmpty(metadata.title)) throw new Error("missing_title");
  if (_.isEmpty(metadata.artist)) throw new Error("missing_artist");
  if (_.isEmpty(metadata.channel)) throw new Error("missing_channel");
  if (!_.isFinite(metadata.sampleStart)) throw new Error("invalid_samplestart");
  if (!_.isFinite(metadata.sampleLength))
    throw new Error("invalid_samplelength");
  if (_.isEmpty(charts)) throw new Error("no_charts");
};
