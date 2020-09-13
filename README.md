# piuGBA

This is a PIU emulator for the GBA that uses [StepMania](https://github.com/stepmania/stepmania) SSC charts.

![demo1](img/1.gif)![demo2](img/2.gif)![demo3](img/3.gif)![demo4](img/4.gif)![demo5](img/5.gif)![demo6](img/6.gif)![demo7](img/7.gif)![demo8](img/8.gif)![demo9](img/9.gif)![demo10](img/10.gif)![demo11](img/11.gif)![demo12](img/12.gif)

## Key features

- Full **.ssc files** support, including:
  * Normal, hold and fake notes
  * BPM changes
  * Scroll speed changes
  * Stops/Delays and async-stops
  * Warps and fast-BPM warps
- Multiple **game modes**:
  * Campaign: Play, unlock songs and defeat bosses
  * Arcade: Play songs in any numerical difficulty level
  * Impossible: Hardcore charts with insane mods
- **Speed multipliers** can be changed in-game
- **Mods** support:
  * Stage break
  * Pixelate: Mosaic effect
  * Jump/Reduce: Moves game area
  * Negative: Inverts palette
  * Random speed
  * Mirror and random steps
  * Extra judgement
- **BGA DARK** background with blink effect
- **Song selector** with names, backgrounds and sound previews
- **Optimized** to support ~60 songs per ROM file.

## How does it work?

A node.js script (the **importer**) converts a list of SSC/MP3/PNG files into binary files which the GBA can understand. For audio, it uses GSM audio files which are very small in size.

Charts are converted into a format created for this project called **PIUS**. Then everything is bundled in a **GBFS** file (a filesystem created by the GBA scene) and appended to the final ROM.

[See the wiki](https://github.com/rodri042/piugba/wiki) for more details!

## How to a build a ROM

- Install everything (read the section below).
- Create in `src/data/content/songs` one folder per song, including:
  - one `.mp3` file with the song
  - one `.png` file with the background
  - one `.ssc` file with the charts
- Run:
```bash
make import
make assets
make restart ENV=production
```

Full guide -> [Wiki: Building a ROM](https://github.com/rodri042/piugba/wiki/Building-a-ROM).

## Install

### Windows

- Use this file structure in `D:\work\gba`:
	* `gba`
		* `tools`
			* `devkitPro`
		* `projects`
			* `piugba`
- Install the toolchain:
  * [VSCode](https://code.visualstudio.com): The IDE
  * [Git Bash](https://gitforwindows.org): The console
  * [devkitPro](https://github.com/devkitPro/installer/releases): The devkit for compiling GBA roms. It comes with:
    * *grit*: Used to convert paletted bitmaps to C arrays or raw binary files
    * *gbfs*: Used to create a package with all the game assets
  * [node.js >= 10](https://nodejs.org/en): The JS runtime
  * [make](scripts/toolchain/programs/make-3.81.zip): The build automation tool
  * [ImageMagick](scripts/toolchain/programs/ImageMagick-7.0.10-3-Q16-x64-static.exe): The tool used to convert images to paletted bitmaps
  * [pngfix](scripts/toolchain/programs/pngfix.exe): A small command line util to fix corrupted PNG files.
  * [ffmpeg *(with libgsm)*](scripts/toolchain/programs/ffmpeg-3.3.3-win64-static.zip): The tool used to convert audio files to PCM
    * To avoid using the `ffmpeg.exe` binary included with *ImageMagick*, add it to `PATH` first!
- Add to `~/.bash_profile`:
```bash
export PATH=$PATH:/d/work/gba/tools/devkitPro/bin
export PATH=$PATH:/d/work/gba/tools/devkitPro/devkitARM/bin
export PATH=$PATH:/d/work/gba/tools/devkitPro/tools/bin
```
- You can check if the tools are installed correctly running `./scripts/toolchain/check.sh`

### VSCode

- Recommended plugins: `C/C++ Extensions`, `EditorConfig`, `Prettier - Code formatter`
- Recommended settings: [here](scripts/toolchain/vscode_settings.json)

## Actions

### Commands

- `make clean`: Cleans build artifacts
- `make assets`: Compiles the needed assets in `src/data/content/_compiled_sprites` (required for compiling)
- `make build`: Compiles and generates a `.gba` file without data
- `make import`: Imports the songs from `src/data/content/songs` to a GBFS file
- `make package`: Compiles and appends the GBFS file to the ROM
- `make start`: Starts the compiled ROM
- `make restart`: Recompiles and starts the ROM
- `make reimport`: Reimport the songs and starts the ROM without recompiling

#### Parameters

Name | Values | Description
--- | --- | ---
`MODE` | **`auto`** or `manual` | When using `auto`, the import process tries to guess the missing data (e.g. difficulty levels). See [Wiki: Autoimporting songs](https://github.com/rodri042/piugba/wiki/Autoimporting-songs).
`SORT` | **`level`** or `dir` | When using `level`, the import process sorts the songs by level, in ascending order. See [Wiki: Song order](https://github.com/rodri042/piugba/wiki/Song-order).
`ENV` | **`development`**, or `debug` or `production` |`debug`: everything is unlocked, backgrounds are disabled, and stage-break is OFF.<br><br> `development`: the same, but including backgrounds.<br><br>`production`: backgrounds, stage-break ON, and working locks.<br><br>Non-production versions also have a *debug menu* to correct songs' offsets. See [Wiki: Correcting offsets](https://github.com/rodri042/piugba/wiki/Building-a-ROM#correcting-offsets).
`SONGS` | *path to a directory* | Songs directory. Defaults to: `src/data/content/songs`

### Scripts

#### Build sprites

```bash
# use #FF00FF as transparency color
grit *.bmp -ftc -pS -gB8 -gT ff00ff -O shared_palette.c
```

#### Build backgrounds

```bash
magick file.png -resize 240x160\! -colors 255 file.bmp
grit file.bmp -gt -gB8 -mRtf -mLs -ftb
```

#### Build music

```bash
ffmpeg -y -i file.mp3 -ac 1 -af 'aresample=18157' -strict unofficial -c:a gsm file.gsm
ffplay -ar 18157 file.gsm
```

#### Build filesystem

```bash
gbfs files.gbfs *.pius *.gsm *.bin
# pad rom.gba to a 256-byte boundary
cat rom.gba files.gbfs > rom.out.gba
```

#### Build gba-sprite-engine

```bash
rm -rf cmake-build-debug ; mkdir cmake-build-debug ; cd cmake-build-debug ; cmake ./../ -G "Unix Makefiles" ; make ; cp engine/libgba-sprite-engine.a ../../piugba/libs/libgba-sprite-engine/lib/libgba-sprite-engine.a ; cd ../
```

### Troubleshooting

#### Undefined reference to *function name*

If you've added new folders, check if they're in `Makefile`'s `SRCDIRS` list!

## Open-source projects involved

- [wgroeneveld/gba-sprite-engine](https://github.com/wgroeneveld/gba-sprite-engine): Dec 18, 2019
  * Forked at: [rodri042/gba-sprite-engine](https://github.com/rodri042/gba-sprite-engine)
- [pinobatch/gsmplayer-gba](https://github.com/pinobatch/gsmplayer-gba): Feb 9, 2020
