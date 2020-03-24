const SongChart = require("./SongChart");
const fs = require("fs");

const path = "../../../src/data/content/csikos/736.ssc";

const content = fs.readFileSync(path).toString();
const songChart = new SongChart(content);
console.log(songChart.metadata);
