const util = require("util");
const childProcess = require("child_process");
const exec = util.promisify(childProcess.exec);
const execSync = (...args) => {
  return { stdout: childProcess.execSync(...args) };
};
const readlineSync = require("readline-sync");
const {
  printTableAndGetConsoleOutput,
} = require("console-table-printer/dist/src/internalTable/internal-table-printer");
const {
  TableInternal,
} = require("console-table-printer/dist/src/internalTable/internal-table");
const _ = require("lodash");

module.exports = {
  run: (command, options) =>
    (GLOBAL_OPTIONS.fast ? exec : execSync)(command, {
      stdio: "ignore",
      shell: true,
      ...options,
    }),
  async report(action, taskName, omitIfAsync = false) {
    const shouldOmit = GLOBAL_OPTIONS.fast && omitIfAsync;
    try {
      const output = await action();
      if (!shouldOmit) console.log(`  ✔️  ${taskName}`.green);
      return output;
    } catch (e) {
      console.log(`  ❌  ${taskName}\n`.red);
      throw e;
    }
  },
  insistentChoice(text, options, textColor = "black") {
    const stringOptions = options.map((it) => `${it}`.toLowerCase());

    let response = "";
    const matches = (option) => _.startsWith(option, response);
    while (response === "" || _.filter(stringOptions, matches).length !== 1)
      response = readlineSync
        .question(`${text}`[textColor].bgWhite + " ")
        .toLowerCase();

    return _.find(stringOptions, matches);
  },
  replaceRange(input, search, replace, start, end = input.length) {
    return (
      input.slice(0, start) +
      input.slice(start, end).replace(search, replace) +
      input.slice(end)
    );
  },
  restrictTo(value, min, max) {
    return Math.max(Math.min(value, max), min);
  },
  printTable(rows) {
    const table = new TableInternal();
    table.addRows(rows);
    table.sortFunction = (a, b) => a.id - b.id;
    printTableAndGetConsoleOutput(table);
  },
};
