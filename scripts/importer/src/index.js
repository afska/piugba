const Simfile = require("./parser/Simfile");
const SongSerializer = require("./serializer/SongSerializer");
const fs = require("fs");

const path = "../../src/data/content/test/chart.ssc";

const content = fs.readFileSync(path).toString();
const simfile = new Simfile(content);

simfile.charts = simfile.charts.filter((it) => it.header.level === 7);

console.log(new SongSerializer(simfile).serialize());
