const Simfile = require("./Simfile");
const fs = require("fs");

const path = "../../src/data/content/csikos/736.ssc";

const content = fs.readFileSync(path).toString();
const simfile = new Simfile(content);
console.log(JSON.stringify(simfile.charts, null, 2));
