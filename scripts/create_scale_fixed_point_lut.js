// float -> 8.8 fixed-point int
function floatTo88Fp(floatValue) {
  let fixedPoint = Math.round(floatValue * 256);
  fixedPoint = Math.max(-32768, Math.min(32767, fixedPoint));
  return fixedPoint;
}

const VALUES = [1.0, 1.1, 1.2, 1.3, 1.5, 1.25];

console.log(VALUES.map((it) => 1 / it).map(floatTo88Fp));
