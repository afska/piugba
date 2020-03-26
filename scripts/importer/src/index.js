const Simfile = require("./parser/Simfile");
const SongSerializer = require("./serializer/SongSerializer");
const fs = require("fs");

const INPUT_FILE = "../../src/data/content/test/chart.ssc";
const OUTPUT_FILE = "../../src/data/content/chart.pius";

const content = fs.readFileSync(INPUT_FILE).toString();
const simfile = new Simfile(content);

simfile.charts = simfile.charts.filter((it) => it.header.level === 7);
const output = new SongSerializer(simfile).serialize();

fs.writeFileSync(OUTPUT_FILE, output);
