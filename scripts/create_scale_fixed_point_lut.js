const _ = require("./importer/node_modules/lodash");

// float -> 8.8 fixed-point int
function floatTo88Fp(floatValue) {
  let fixedPoint = Math.round(floatValue * 256);
  fixedPoint = Math.max(-32768, Math.min(32767, fixedPoint));
  return fixedPoint;
}

const VALUES = {
  score: [1.0, 1.1, 1.2, 1.3, 1.5, 1.25],
  breath: [
    1.0, 1.05, 1.1, 1.15, 1.2, 1.25, 1.225, 1.2, 1.175, 1.15, 1.125, 1.1, 1.075,
    1.05, 1.025,
  ],
};

console.log(
  _.mapValues(VALUES, (values) => values.map((it) => 1 / it).map(floatTo88Fp))
);
