const childProcess = require("child_process");
const readlineSync = require("readline-sync");

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
  insistentPrompt(text, type = "question") {
    let response = "";
    while (response === "") response = readlineSync[type]`${text} `;
    return response;
  },
};
