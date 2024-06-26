const fs = require("fs");
const childProcess = require("child_process");
const $path = require("path");

const ROOT_DIR = $path.join(__dirname, "..");
const CONTENT_DIR = $path.join(ROOT_DIR, "src/data/content");
const SONG_PACKS_DIR = $path.join(CONTENT_DIR, "songs-pack");
const ROM_PACKS_DIR = $path.join(CONTENT_DIR, "roms");
const DEV_DIR = $path.join(CONTENT_DIR, "roms/#dev");
const FLIPS_DIR = $path.join(__dirname, "toolchain/programs/flips");
const FLIPS_TO_CONTENT_PATH = "../../../../src/data/content";
const ROMNAME = "romname.txt";
const ARCADE_SIGNAL = "ARCADE";
const VARIANTS = ["arcade", "full"];
const ENVIRONMENTS = ["development", "production"];
const OUTPUT_EMPTY = "piugba.gba";
const OUTPUT_FULL = "piugba.out.gba";
const OUTPUT_BUILDS = (variant) => ({
  development: `piugba.${variant}.dev.gba`,
  production: `piugba.${variant}.prod.gba`,
});
const ARCADE_FLAG = (variant) => (variant === "arcade" ? "ARCADE=true" : "");
const SEPARATOR = "----------";

const SEARCH = process.argv[2];

const log = (text) => console.log(`${SEPARATOR}${text}${SEPARATOR}`);
const run = (command, options) => {
  return childProcess.execSync(command, {
    stdio: ["pipe", process.stdout, process.stderr],
    shell: true,
    ...options,
  });
};
const getPatches = () => {
  try {
    const output = run("git --no-pager branch", {
      cwd: ROOT_DIR,
      stdio: null,
    }).toString();
    return output
      .split("\n")
      .map((it) => it.trim().replace(/^\* /, ""))
      .filter((it) => it.startsWith("patch_"));
  } catch (e) {
    console.error("Failed to get patches", e);
    process.exit(1);
  }
};

// -----------
// COMPILATION
// -----------

if (!SEARCH) {
  const patches = getPatches();

  VARIANTS.forEach((variant) => {
    ENVIRONMENTS.forEach((environment) => {
      log(`⌚  COMPILING: VARIANT=${variant}, ENV=${environment}`);
      run("git checkout master", { cwd: ROOT_DIR });
      run("make clean", { cwd: ROOT_DIR });
      run("make assets", { cwd: ROOT_DIR });
      run(`make build ENV="${environment}" ${ARCADE_FLAG(variant)}`, {
        cwd: ROOT_DIR,
      });
      fs.copyFileSync(
        $path.join(ROOT_DIR, OUTPUT_EMPTY),
        $path.join(CONTENT_DIR, OUTPUT_BUILDS(variant)[environment])
      );

      if (environment === "production") {
        patches.forEach((patch) => {
          const outputName = OUTPUT_BUILDS(variant)[environment].replace(
            "piugba",
            patch
          );

          log(
            `⌚  COMPILING: VARIANT=${variant}, ENV=${environment}, PATCH=${patch}`
          );
          run(`git checkout ${patch}`, { cwd: ROOT_DIR });
          run("make clean", { cwd: ROOT_DIR });
          run("make assets", { cwd: ROOT_DIR });
          run(`make build ENV="${environment}" ${ARCADE_FLAG(variant)}`, {
            cwd: ROOT_DIR,
          });
          fs.copyFileSync(
            $path.join(ROOT_DIR, OUTPUT_EMPTY),
            $path.join(CONTENT_DIR, outputName)
          );
          run("git checkout master", { cwd: ROOT_DIR });
        });
      }
    });
  });

  patches.forEach((patch) => {
    VARIANTS.forEach((variant) => {
      const cleanFile = OUTPUT_BUILDS(variant).production;
      const patchedFile = OUTPUT_BUILDS(variant).production.replace(
        "piugba",
        patch
      );

      log(
        `⌚  BUILDING IPS PATCH: VARIANT=${variant}, ENV=production, PATCH=${patch}`
      );

      run(
        `flips.exe --create --ips ${FLIPS_TO_CONTENT_PATH}/${cleanFile} ${FLIPS_TO_CONTENT_PATH}/${patchedFile} ${FLIPS_TO_CONTENT_PATH}/${patch}.${variant}.prod.ips`,
        {
          cwd: FLIPS_DIR,
        }
      );
      run(`rm ${patchedFile}`, { cwd: CONTENT_DIR });
    });
  });
}

// ---------
// IMPORTING
// ---------

const sources = fs
  .readdirSync(SONG_PACKS_DIR)
  .filter((it) => !it.startsWith("."))
  .filter((it) => !it.startsWith("#"))
  .filter((it) => (SEARCH != null ? it.startsWith(`(${SEARCH})`) : true))
  .map((it) => ({
    name: it,
    path: $path.join(SONG_PACKS_DIR, it),
    variant: it.includes(ARCADE_SIGNAL) ? "arcade" : "full",
  }));

sources.forEach(({ name, path, variant }) => {
  const prefix = name.split(" ")[0];
  const shortName = fs.readFileSync($path.join(path, ROMNAME)).toString();
  log(`⌚  IMPORTING: ${name} <<${shortName}>>`);

  run(`make import SONGS="${path}" ${ARCADE_FLAG(variant)} FAST=true`, {
    cwd: ROOT_DIR,
  });

  ENVIRONMENTS.forEach((environment) => {
    fs.copyFileSync(
      $path.join(CONTENT_DIR, OUTPUT_BUILDS(variant)[environment]),
      $path.join(ROOT_DIR, OUTPUT_EMPTY)
    );
    run("make package", { cwd: ROOT_DIR });
    const outputName = `${prefix} piuGBA - ${shortName}.gba`;
    if (environment === "production") {
      fs.copyFileSync(
        $path.join(ROOT_DIR, OUTPUT_FULL),
        $path.join(ROM_PACKS_DIR, name, outputName)
      );
    } else {
      fs.copyFileSync(
        $path.join(ROOT_DIR, OUTPUT_FULL),
        $path.join(DEV_DIR, outputName)
      );
    }
  });

  console.log(`✔️  ${prefix} ${shortName}`);
});
