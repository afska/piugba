const _ = require("./importer/node_modules/lodash");

const count = (str, ch) => _.countBy(str)[ch] || 0;
const lookUpTable = _.range(0, 32).map((it) => count(it.toString(2), "1"))
// _.range(0, 32).filter((it) => count(it.toString(2), "1") == i)

console.log(JSON.stringify(lookUpTable));
