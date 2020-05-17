const _ = require("./importer/node_modules/lodash");

const DIGITS = 3;
const MIN_COMBO = 0;
const MAX_COMBO = 999;

const range = _.range(MIN_COMBO, MAX_COMBO + 1);
const lookUpTable = _.flatMap(range, (it) => {
  const [d1, d2, d3] = _.padStart(it, DIGITS, 0);
  return [d1, d2, d3].map((it) => parseInt(it));
});

console.log(JSON.stringify(lookUpTable));
