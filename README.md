# piugba

This is a version of PIU for the GBA. It's under development, so don't try to compile it yet: it won't work since the necessary images and audio files (`src/data/content`) are not here. A ROM creator will be provided in the final release.

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
- `make assets`: Compiles the needed assets in `src/data/content/compiled` (required for compiling)
- `make build`: Compiles and generates a `.gba` file without data
- `make import`: Imports the songs from `src/data/content/songs` to a GBFS file
- `make package`: Compiles and appends the GBFS file to the ROM
- `make start`: Starts the compiled ROM
- `make restart`: Recompiles and starts the ROM

### Scripts

#### Build sprites

```bash
# use #FF00FF as transparency color
grit *.bmp -ftc -pS -gB8 -gT ff00ff -O shared_palette.c
```

#### Build backgrounds

```bash
magick file.png -resize 240x160\! -colors 255 file.bmp
grit file.bmp -gt -gB8 -mR! -mLs -ftb
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

#### Log numbers

```cpp
#include <libgba-sprite-engine/background/text_stream.h>
log_text(std::to_string(number).c_str());
```

#### Undefined reference to *function name*

If you've added new folders, check if they're in `Makefile`'s `SRCDIRS` list!

## Open-source projects involved

- [wgroeneveld/gba-sprite-engine](https://github.com/wgroeneveld/gba-sprite-engine): Dec 18, 2019
  * Forked at: [rodri042/gba-sprite-engine](https://github.com/rodri042/gba-sprite-engine)
- [pinobatch/gsmplayer-gba](https://github.com/pinobatch/gsmplayer-gba): Feb 9, 2020
