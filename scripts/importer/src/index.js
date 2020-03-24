const Simfile = require("./Simfile");
const fs = require("fs");

const path = "../../src/data/content/test/chart.ssc";

const content = fs.readFileSync(path).toString();
const simfile = new Simfile(content);
console.log(
  JSON.stringify(
    simfile.charts.find((it) => it.header.level == 7),
    null,
    2
  )
);
