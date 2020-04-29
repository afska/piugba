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
      const output = action() || "";
      console.log(`  ✔️  ${taskName}${output}`.green);
    } catch (e) {
      console.log(`  ❌  ${taskName}\n`.red);
      throw e;
    }
  },
  insistentChoice(text, options) {
    const stringOptions = options.map((it) => `${it}`.toLowerCase());

    let response = "";
    while (response === "" || !_.includes(stringOptions, response))
      response = readlineSync.question(`${text}`.black.bgWhite + " ");

    return response;
  },
};
