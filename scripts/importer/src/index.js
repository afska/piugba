const Simfile = require("./parser/Simfile");
const SongSerializer = require("./serializer/SongSerializer");
const childProcess = require("child_process");
const fs = require("fs");
const mkdirp = require("mkdirp");
const $path = require("path");
const _ = require("lodash");

const SONGS_PATH = `${__dirname}/../../../src/data/content/songs`;
const OUTPUT_PATH = `${SONGS_PATH}/../#compiled_songs`;
const GBFS_PATH = `${OUTPUT_PATH}/../files.gbfs`;

const COMMAND_AUDIO = (input, output) =>
  `ffmpeg -y -i "${input}" -ac 1 -af aresample=18157 -strict unofficial -c:a gsm "${output}"`;
const COMMAND_BACKGROUND_1 = (input, output) =>
  `magick "${input}" -resize 240x160\! -colors 255 "${output}"`;
const COMMAND_BACKGROUND_2 = (input) =>
  `grit "${input}" -gt -gB8 -mRtf -mLs -ftb`;
const COMMAND_BACKGROUND_3 = (tmp1, tmp2) => `rm "${tmp1}" && rm "${tmp2}"`;

const FILE_METADATA = /\.ssc/i;
const FILE_AUDIO = /\.mp3/i;
const FILE_BACKGROUND = /\.png/i;
const EXTENSION_METADATA = "pius";
const EXTENSION_AUDIO = "gsm";
const EXTENSION_BACKGROUND_TMP1 = "bmp";
const EXTENSION_BACKGROUND_TMP2 = "h";

const run = (command, options) =>
  childProcess.execSync(command, { shell: true, ...options });

mkdirp(SONGS_PATH);
run(`rm -rf ${OUTPUT_PATH}`);
mkdirp.sync(OUTPUT_PATH);

fs.readdirSync(SONGS_PATH)
  .map((directory) => {
    return {
      path: $path.join(SONGS_PATH, directory),
      name: _(directory).split(" - ").last(),
    };
  })
  .forEach(({ path, name }) => {
    const files = fs.readdirSync(path).map((it) => $path.join(path, it));
    const metadataFile = _.find(files, (it) => FILE_METADATA.test(it));
    const audioFile = _.find(files, (it) => FILE_AUDIO.test(it));
    const backgroundFile = _.find(files, (it) => FILE_BACKGROUND.test(it));

    // metadata
    const content = fs.readFileSync(metadataFile).toString();
    const simfile = new Simfile(content, name);
    const output = new SongSerializer(simfile).serialize();
    fs.writeFileSync(
      $path.join(OUTPUT_PATH, `${name}.${EXTENSION_METADATA}`),
      output
    );

    // audio
    run(
      COMMAND_AUDIO(
        audioFile,
        $path.join(OUTPUT_PATH, `${name}.${EXTENSION_AUDIO}`)
      )
    );

    // background
    const tempFile1 = $path.join(
      OUTPUT_PATH,
      `${name}.${EXTENSION_BACKGROUND_TMP1}`
    );
    const tempFile2 = $path.join(
      OUTPUT_PATH,
      `${name}.${EXTENSION_BACKGROUND_TMP2}`
    );
    run(COMMAND_BACKGROUND_1(backgroundFile, tempFile1));
    run(COMMAND_BACKGROUND_2(tempFile1), { cwd: OUTPUT_PATH });
    run(COMMAND_BACKGROUND_3(tempFile1, tempFile2));
  });
