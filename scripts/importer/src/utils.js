const childProcess = require("child_process");

module.exports = {
  run: (command, options) =>
    childProcess.execSync(command, {
      stdio: "ignore",
      shell: true,
      ...options,
    }),
  report(action, taskName) {
    try {
      action();
      console.log(`  ✔️  ${taskName}`.green);
    } catch (e) {
      console.log(`  ❌  ${taskName}\n`.red);
      throw e;
    }
  },
};
