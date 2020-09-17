const _ = require("./importer/node_modules/lodash");

const MIN_LIFE = 0;
const MAX_LIFE = 100;
const MAX_MOSAIC = 4;

const lookUpTables = {
  lifebar: _
  .range(MIN_LIFE, MAX_LIFE + 1)
  .map((it) => Math.trunc(it / 10)),
  mosaic: _.range(MIN_LIFE, MAX_LIFE + 1).map((it) => Math.round(
    MAX_MOSAIC - it * MAX_MOSAIC / MAX_LIFE
  ))
};

console.log(JSON.stringify(lookUpTables));
