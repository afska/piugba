const _ = require("./importer/node_modules/lodash");

// float -> 8.8 fixed-point int
function floatTo88Fp(floatValue) {
  let fixedPoint = Math.round(floatValue * 256);
  fixedPoint = Math.max(-32768, Math.min(32767, fixedPoint));
  return fixedPoint;
}

const VALUES = {
  score: [1.0, 1.1, 1.2, 1.3, 1.5, 1.25],
  breath: [],
};

for (let i = 0; i < 9; i++) VALUES.breath.push(1 + (0.25 * i) / 10);
for (let i = 0; i < 9; i++) VALUES.breath.push(1.225 - (0.25 * (1 + i)) / 10);
VALUES.breath.pop();

console.log(
  _.mapValues(VALUES, (values) => values.map((it) => 1 / it).map(floatTo88Fp))
);
