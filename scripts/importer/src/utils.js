const childProcess = require("child_process");
const readlineSync = require("readline-sync");
const _ = require("lodash");

module.exports = {
  run: (command, options) =>
    childProcess.execSync(command, {
      stdio: "ignore",
      shell: true,
      ...options,
    }),
  report(action, taskName) {
    try {
      const output = action();
      console.log(`  ✔️  ${taskName}`.green);
      return output;
    } catch (e) {
      console.log(`  ❌  ${taskName}\n`.red);
      throw e;
    }
  },
  insistentChoice(text, options, textColor = "black") {
    const stringOptions = options.map((it) => `${it}`.toLowerCase());

    let response = "";
    while (response === "" || !_.includes(stringOptions, response))
      response = readlineSync.question(`${text}`[textColor].bgWhite + " ");

    return response.toLowerCase();
  },
  replaceRange(input, search, replace, start, end = input.length) {
    return (
      input.slice(0, start) +
      input.slice(start, end).replace(search, replace) +
      input.slice(end)
    );
  },
};
