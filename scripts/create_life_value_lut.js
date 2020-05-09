const _ = require("./importer/node_modules/lodash");

const MIN_LIFE = 0;
const MAX_LIFE = 100;

const lookUpTable = _
  .range(MIN_LIFE, MAX_LIFE + 1)
  .map((it) => Math.trunc(it / 10));

console.log(JSON.stringify(lookUpTable));
